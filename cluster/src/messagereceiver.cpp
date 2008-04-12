/*
 * messagereceiver.cpp
 *
 * Copyright (C) 2006 - 2008 by Universitaet Stuttgart (VIS). 
 * Alle Rechte vorbehalten.
 */

#include "messagereceiver.h"

#include "vislib/assert.h"
#include "vislib/clustermessages.h"
#include "vislib/IllegalParamException.h"
#include "vislib/RawStorage.h"
#include "vislib/SocketException.h"
#include "vislib/Trace.h"



/*
 * vislib::net::cluster::ReceiveMessages
 */
DWORD vislib::net::cluster::ReceiveMessages(void *receiveMessagesCtx) {
    ReceiveMessagesCtx *ctx = static_cast<ReceiveMessagesCtx *>(
        receiveMessagesCtx);            // Receive context.
    const MessageHeader *msgHdr = NULL; // Pointer to header area of 'recvBuf'.
    const BlockHeader *blkHdr = NULL;   // Pointer to a block header.
    SIZE_T msgSize = 0;                 // Total message size (header + body);
    RawStorage recvBuf;                 // Receives data from network.
    DWORD retval = 0;                   // Function return value.

    /* Sanity checks. */
    ASSERT(ctx != NULL);
    ASSERT(ctx->Receiver != NULL);
    ASSERT(ctx->Socket != NULL);
    if ((ctx == NULL) || (ctx->Receiver == NULL) || (ctx->Socket == NULL)) {
        throw IllegalParamException("receiveMessagesCtx", __FILE__, __LINE__);
    }

    try {

        TRACE(Trace::LEVEL_VL_INFO, "The cluster message receiver thread is "
            "starting ...\n");

        while (true) {

            /* Receive a message header into 'recvBuf'. */
            recvBuf.AssertSize(sizeof(MessageHeader));
            msgHdr = recvBuf.As<MessageHeader>();
            ctx->Socket->Receive(static_cast<void *>(recvBuf), 
                sizeof(MessageHeader), Socket::TIMEOUT_INFINITE, 0, true);

            /* Sanity check. */
            ASSERT(recvBuf.GetSize() >= sizeof(MessageHeader));
            //ASSERT(msgHdr->MagicNumber == MAGIC_NUMBER);
            if (msgHdr->MagicNumber != MAGIC_NUMBER) {
                TRACE(Trace::LEVEL_WARN, "Discarding data packet without valid "
                    "magic number. Expected %u, but received %u.\n",
                    MAGIC_NUMBER, msgHdr->MagicNumber);
                break;
            }

            /* Receive the rest of the message after the header in 'recvBuf'. */
            msgSize = sizeof(MessageHeader) + msgHdr->Header.BlockLength;
            recvBuf.AssertSize(msgSize, true);
            msgHdr = recvBuf.As<MessageHeader>();
            ctx->Socket->Receive(recvBuf.As<BYTE>() + sizeof(MessageHeader),
                msgHdr->Header.BlockLength, Socket::TIMEOUT_INFINITE, 0, true);

            /* Call the handler method to process the message. */
            if (msgHdr->Header.BlockId == MSGID_MULTIPLE) {
                /* Received a compound message, so split it. */
                TRACE(Trace::LEVEL_VL_INFO, "Splitting compond message ...\n");
                INT remBody = static_cast<INT>(msgHdr->Header.BlockLength);
                const BYTE *d = recvBuf.As<BYTE>() + sizeof(MessageHeader);

                while (remBody > 0) {
                    blkHdr = reinterpret_cast<const BlockHeader *>(d);
                    const BYTE *body = d + sizeof(MessageHeader);

                    TRACE(Trace::LEVEL_VL_INFO, "Received message %u.\n", 
                        blkHdr->BlockId);
                    ctx->Receiver->onMessageReceived(*ctx->Socket,
                        blkHdr->BlockId, 
                        (blkHdr->BlockLength > 0) ? body : NULL,
                        blkHdr->BlockLength);

                    d += blkHdr->BlockLength;
                    remBody -= blkHdr->BlockLength;
                }

            } else {
                /* Receive single message. */
                const BlockHeader *blkHdr = &msgHdr->Header;
                const BYTE *body = recvBuf.As<BYTE>() + sizeof(MessageHeader);

                TRACE(Trace::LEVEL_VL_INFO, "Received message %u.\n", 
                    blkHdr->BlockId);
                ctx->Receiver->onMessageReceived(*ctx->Socket,
                    blkHdr->BlockId, 
                    (blkHdr->BlockLength > 0) ? body : NULL,
                    blkHdr->BlockLength);
            }
        } /* end while (true) */
    } catch (SocketException e) {
        TRACE(Trace::LEVEL_VL_ERROR, "vislib::net::cluster::ReceiveMessages "
            "exits because of communication error: %s\n", e.GetMsgA());
        retval = e.GetErrorCode();
    } catch (...) {
        TRACE(Trace::LEVEL_VL_ERROR, "Unexpected exception caught in "
            "vislib::net::cluster::ReceiveMessages.\n");
        retval = -1;
    }

    SAFE_DELETE(ctx);
    return retval;
}
