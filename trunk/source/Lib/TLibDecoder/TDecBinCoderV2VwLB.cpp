/*! ====================================================================================================================
 * \file
    TDecV2V.cpp
 *  \brief
    Copyright information.
 *  \par Copyright statements
    HEVC (JCTVC cfp)

    This software, including its corresponding source code, object code and executable, may only be used for 
    (1) research purposes or (2) for evaluation for standardisation purposes within the joint collaborative team on 
    video coding for HEVC , provided that this copyright notice and this corresponding notice appear in all copies, 
    and that the name of Research in Motion Limited not be used in advertising or publicity without specific, written 
    prior permission.  This software, as defined above, is provided as a proof-of-concept and for demonstration 
    purposes only; there is no representation about the suitability of this software, as defined above, for any purpose.  
 
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,PROCUREMENT OF SUBSTITUTE GOODS OR 
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
    USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
    
    TRADEMARKS, Product and company names mentioned herein may be the trademarks of their respective owners.  
    Any rights not expressly granted herein are reserved.
 
    Copyright (C) 2010 by Research in Motion Limited, Canada      
    All rights reserved.
 
 *  \par Full Contact Information
    Standards & Licensing Department      (standards-ipr@rim.com)
    Research in Motion
    122 West John Carpenter Parkway
    Irving, TX 75039, USA

 * ====================================================================================================================
*/
 
#include "TDecBinCoderV2VwLB.h"

#include "TDecV2VTrees67k.h"

void TDecV2V::decode_seq(int state, int scl) {

    UInt preflen = 0, phrase = 0;
    UInt bitbuf = 0, bitbufsize = 0;

    bitsUsed = codeBuffer = 0;

    while (1) {
        UInt bit = 0;

        if (preflen > 0) {
            --preflen;
        }
        else if (phrase > 1) {
            if ((phrase & 1) == 0) bit = !bit;
            phrase >>= 1;
        }
        else {

            int pos = 0;
            UInt dpp = dec_tree_complete[state][0];

            while (!(dpp >> 30)) {
                int len = dpp >> 16;
                while (scl && bitsUsed < len) {
                    codeBuffer |= myReadByte() << bitsUsed;
                    bitsUsed += 8;
                    --scl;
                }
                if (!scl && bitsUsed < len) {
                    if (bitbufsize) {
                        temp_space[index++] = (UChar)bitbuf;
                    }
                    bitsUsed = codeBuffer = 0;
                    return;
                }

                pos = (dpp & 0xffff) + (codeBuffer & ((1 << len) - 1));
                codeBuffer >>= len; bitsUsed -= len;
                dpp = dec_tree_complete[state][pos];
            }

            switch (dpp >> 30) {
                case 1:
                    phrase = (1 << ((dpp >> 25) & 0x1f)) | (dpp & 0x1ffffff);
                    if ((phrase & 1) == 0) bit = !bit;
                    phrase >>= 1;    /* Consume next output bit */
                    break;
                case 2:
                    preflen = (dpp & 0xffffff) - 1;
                    phrase = 2;
                    --preflen;    /* Consume next output bit */
                    break;
                case 3:
                    preflen = (dpp & 0xffffff) - 1;
                    phrase = 3;
                    --preflen;    /* Consume next output bit */
                    break;
                /*default:
                    assert(0); */
            }
        }

        bitbuf |= bit << bitbufsize;
        if (++bitbufsize == 8) {
            temp_space[index++] = (UChar)bitbuf;
            bitbuf = bitbufsize = 0;
        }
    }
}
