#ifndef TYPEDEF_H
#define TYPEDEF_H

#include <stdint.h>
#include <stdio.h>
#ifndef OVERRIDE
#ifdef WEBRTC_MAC
#define OVERRIDE override
#else
#define OVERRIDE
#endif//webrtc_mac end
#endif

#define PACK_MAX_LEN 1500
#define UDP_BUF_SIZE PACK_MAX_LEN
#define TCP_BUF_SIZE PACK_MAX_LEN*10

#define MAX_CHANNEL 10

#ifndef NULL
#define NULL 0
#endif

#define SAFE_DELETE(x) {if (x != NULL) {delete x; x = NULL;}}
#define GET_MAX_VALUE(a,b)            (((a) > (b)) ? (a) : (b))
#define GET_MIN_VALUE(a,b)            (((a) < (b)) ? (a) : (b))

#define OUT
#define IN

//will use Thread::Sleep
//#ifndef _WIN32
//#define Sleep(x) usleep((x)*1000)
//#endif

// Processor architecture detection.  For more info on what's defined, see:
//   http://msdn.microsoft.com/en-us/library/b0084kay.aspx
//   http://www.agner.org/optimize/calling_conventions.pdf
//   or with gcc, run: "echo | gcc -E -dM -"
#if defined(_M_X64) || defined(__x86_64__)
#define WEBRTC_ARCH_X86_FAMILY
#define WEBRTC_ARCH_X86_64
#define WEBRTC_ARCH_64_BITS
#define WEBRTC_ARCH_LITTLE_ENDIAN
#elif defined(__aarch64__)
#define WEBRTC_ARCH_64_BITS
#define WEBRTC_ARCH_LITTLE_ENDIAN
#elif defined(_M_IX86) || defined(__i386__)
#define WEBRTC_ARCH_X86_FAMILY
#define WEBRTC_ARCH_X86
#define WEBRTC_ARCH_32_BITS
#define WEBRTC_ARCH_LITTLE_ENDIAN
#elif defined(__ARMEL__)
// TODO(ajm): We'd prefer to control platform defines here, but this is
// currently provided by the Android makefiles. Commented to avoid duplicate
// definition warnings.
//#define WEBRTC_ARCH_ARM
// TODO(ajm): Chromium uses the following two defines. Should we switch?
//#define WEBRTC_ARCH_ARM_FAMILY
//#define WEBRTC_ARCH_ARMEL
#define WEBRTC_ARCH_32_BITS
#define WEBRTC_ARCH_LITTLE_ENDIAN
#elif defined(__MIPSEL__)
#define WEBRTC_ARCH_32_BITS
#define WEBRTC_ARCH_LITTLE_ENDIAN
#elif defined(__pnacl__)
#define WEBRTC_ARCH_32_BITS
#define WEBRTC_ARCH_LITTLE_ENDIAN
#else
#error Please add support for your architecture in typedefs.h
#endif

#if !(defined(WEBRTC_ARCH_LITTLE_ENDIAN) ^ defined(WEBRTC_ARCH_BIG_ENDIAN))
#error Define either WEBRTC_ARCH_LITTLE_ENDIAN or WEBRTC_ARCH_BIG_ENDIAN
#endif

#if (defined(WEBRTC_ARCH_X86_FAMILY) && !defined(__SSE2__)) ||  \
    (defined(WEBRTC_ARCH_ARM_V7) && !defined(WEBRTC_ARCH_ARM_NEON))
#define WEBRTC_CPU_DETECTION
#endif


// Annotate a function indicating the caller must examine the return value.
// Use like:
//   int foo() WARN_UNUSED_RESULT;
// TODO(ajm): Hack to avoid multiple definitions until the base/ of webrtc and
// libjingle are merged.
#if !defined(WARN_UNUSED_RESULT)
#if defined(__GNUC__)
#define WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#define WARN_UNUSED_RESULT
#endif
#endif  // WARN_UNUSED_RESULT

#define SR_UDP_PACK_SIZE 1460
///common struct

typedef struct _tagRawPack
{
    _tagRawPack(){ raw_packet_len_ = 0; }
    char raw_packet_[SR_UDP_PACK_SIZE];
    unsigned short raw_packet_len_;
} RawPacket;


#endif // UDPCONNECTION_H
