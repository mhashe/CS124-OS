/*
 * The Breakfast Club, CS 124, Fall 2017
 * Vaibhav Anand, Nikhil Gupta, Michael Hashe
 *
 * This handles the errors as defined in errno.h
 * It prints the error message to stderr.
 * 
 */

#include <errno.h>
#include <stdio.h>

#include "err_hand.h"


int print_err() {
    int errsv = errno;

    switch(errsv) {
        case E2BIG:
            fprintf(stderr, "Argument list too long.\n");
            break;
        case EACCES:
            fprintf(stderr, "Permission denied.\n");
            break;
        case EADDRINUSE:
            fprintf(stderr, "Address already in use.\n");
            break;
        case EADDRNOTAVAIL:
            fprintf(stderr, "Address not available.\n");
            break;
        case EAFNOSUPPORT:
            fprintf(stderr, "Address family not supported.\n");
            break;
        case EAGAIN:
            fprintf(stderr, "Resource temporarily unavailable.\n");
            break;
        case EALREADY:
            fprintf(stderr, "Connection already in progress.\n");
            break;
        case EBADE:
            fprintf(stderr, "Invalid exchange.\n");
            break;
        case EBADF:
            fprintf(stderr, "Bad file descriptor.\n");
            break;
        case EBADFD:
            fprintf(stderr, "File descriptor in bad state.\n");
            break;
        case EBADMSG:
            fprintf(stderr, "Bad message.\n");
            break;
        case EBADR:
            fprintf(stderr, "Invalid request descriptor.\n");
            break;
        case EBADRQC:
            fprintf(stderr, "Invalid request code.\n");
            break;
        case EBADSLT:
            fprintf(stderr, "Invalid slot.\n");
            break;
        case EBUSY:
            fprintf(stderr, "Device or resource busy.\n");
            break;
        case ECANCELED:
            fprintf(stderr, "Operation canceled.\n");
            break;
        case ECHILD:
            fprintf(stderr, "No child processes.\n");
            break;
        case ECHRNG:
            fprintf(stderr, "Channel number out of range.\n");
            break;
        case ECOMM:
            fprintf(stderr, "Communication error on send.\n");
            break;
        case ECONNABORTED:
            fprintf(stderr, "Connection aborted.\n");
            break;
        case ECONNREFUSED:
            fprintf(stderr, "Connection refused.\n");
            break;
        case ECONNRESET:
            fprintf(stderr, "Connection reset.\n");
            break;
        case EDEADLK:
            fprintf(stderr, "Resource deadlock avoided.\n");
            break;
        case EDESTADDRREQ:
            fprintf(stderr, "Destination address required.\n");
            break;
        case EDOM:
            fprintf(stderr, "Mathematics argument out of domain of function.\n");
            break;
        case EDQUOT:
            fprintf(stderr, "Disk quota exceeded.\n");
            break;
        case EEXIST:
            fprintf(stderr, "File exists.\n");
            break;
        case EFAULT:
            fprintf(stderr, "Bad address.\n");
            break;
        case EFBIG:
            fprintf(stderr, "File too large.\n");
            break;
        case EHOSTDOWN:
            fprintf(stderr, "Host is down.\n");
            break;
        case EHOSTUNREACH:
            fprintf(stderr, "Host is unreachable.\n");
            break;
        case EIDRM:
            fprintf(stderr, "Identifier removed.\n");
            break;
        case EILSEQ:
            fprintf(stderr, 
                "Invalid or incomplete multibyte or wide character.\n");
            break;
        case EINPROGRESS:
            fprintf(stderr, "Operation in progress.\n");
            break;
        case EINTR:
            fprintf(stderr, "Interrupted function call.\n");
            break;
        case EINVAL:
            fprintf(stderr, "Invalid argument.\n");
            break;
        case EIO:
            fprintf(stderr, "Input/output error.\n");
            break;
        case EISCONN:
            fprintf(stderr, "Socket is connected.\n");
            break;
        case EISDIR:
            fprintf(stderr, "Is a directory.\n");
            break;
        case EISNAM:
            fprintf(stderr, "Is a named type file.\n");
            break;
        case EKEYEXPIRED:
            fprintf(stderr, "Key has expired.\n");
            break;
        case EKEYREJECTED:
            fprintf(stderr, "Key was rejected by service.\n");
            break;
        case EKEYREVOKED:
            fprintf(stderr, "Key has been revoked.\n");
            break;
        case EL2HLT:
            fprintf(stderr, "Level 2 halted.\n");
            break;
        case EL2NSYNC:
            fprintf(stderr, "Level 2 not synchronized.\n");
            break;
        case EL3HLT:
            fprintf(stderr, "Level 3 halted.\n");
            break;
        case EL3RST:
            fprintf(stderr, "Level 3 reset.\n");
            break;
        case ELIBACC:
            fprintf(stderr, "Cannot access a needed shared library.\n");
            break;
        case ELIBBAD:
            fprintf(stderr, "Accessing a corrupted shared library.\n");
            break;
        case ELIBMAX:
            fprintf(stderr, 
                "Attempting to link in too many shared libraries.\n");
            break;
        case ELIBSCN:
            fprintf(stderr, ".lib section in a.out corrupted\n");
            break;
        case ELIBEXEC:
            fprintf(stderr, "Cannot exec a shared library directly.\n");
            break;
        case ELOOP:
            fprintf(stderr, "Too many levels of symbolic links.\n");
            break;
        case EMEDIUMTYPE:
            fprintf(stderr, "Wrong medium type.\n");
            break;
        case EMFILE:
            fprintf(stderr, "Too many open files.\n");
            break;
        case EMLINK:
            fprintf(stderr, "Too many links.\n");
            break;
        case EMSGSIZE:
            fprintf(stderr, "Message too long.\n");
            break;
        case EMULTIHOP:
            fprintf(stderr, "Multihop attempted.\n");
            break;
        case ENAMETOOLONG:
            fprintf(stderr, "Filename too long.\n");
            break;
        case ENETDOWN:
            fprintf(stderr, "Network is down.\n");
            break;
        case ENETRESET:
            fprintf(stderr, "Connection aborted by network.\n");
            break;
        case ENETUNREACH:
            fprintf(stderr, "Network unreachable.\n");
            break;
        case ENFILE:
            fprintf(stderr, "Too many open files in system.\n");
            break;
        case ENOBUFS:
            fprintf(stderr, "No buffer space available.\n");
            break;
        case ENODATA:
            fprintf(stderr, 
                "No message is available on the STREAM head read queue.\n");
            break;
        case ENODEV:
            fprintf(stderr, "No such device.\n");
            break;
        case ENOENT:
            fprintf(stderr, "No such file or directory.\n");
            break;
        case ENOEXEC:
            fprintf(stderr, "Exec format error.\n");
            break;
        case ENOKEY:
            fprintf(stderr, "Required key not available.\n");
            break;
        case ENOLCK:
            fprintf(stderr, "No locks available.\n");
            break;
        case ENOLINK:
            fprintf(stderr, "Link has been severed.\n");
            break;
        case ENOMEDIUM:
            fprintf(stderr, "No medium found.\n");
            break;
        case ENOMEM:
            fprintf(stderr, "Not enough space.\n");
            break;
        case ENOMSG:
            fprintf(stderr, "No message of the desired type.\n");
            break;
        case ENONET:
            fprintf(stderr, "Machine is not on the network.\n");
            break;
        case ENOPKG:
            fprintf(stderr, "Package not installed.\n");
            break;
        case ENOPROTOOPT:
            fprintf(stderr, "Protocol not available.\n");
            break;
        case ENOSPC:
            fprintf(stderr, "No space left on device.\n");
            break;
        case ENOSR:
            fprintf(stderr, "No STREAM resources.\n");
            break;
        case ENOSTR:
            fprintf(stderr, "Not a STREAM.\n");
            break;
        case ENOSYS:
            fprintf(stderr, "Function not implemented.\n");
            break;
        case ENOTBLK:
            fprintf(stderr, "Block device required.\n");
            break;
        case ENOTCONN:
            fprintf(stderr, "The socket is not connected.\n");
            break;
        case ENOTDIR:
            fprintf(stderr, "Not a directory.\n");
            break;
        case ENOTEMPTY:
            fprintf(stderr, "Directory not empty.\n");
            break;
        case ENOTSOCK:
            fprintf(stderr, "Not a socket.\n");
            break;
        case ENOTTY:
            fprintf(stderr, "Inappropriate I/O control operation.\n");
            break;
        case ENOTUNIQ:
            fprintf(stderr, "Name not unique on network.\n");
            break;
        case ENXIO:
            fprintf(stderr, "No such device or address.\n");
            break;
        case EOPNOTSUPP:
            fprintf(stderr, "Operation not supported on socket.\n");
            break;
        case EOVERFLOW:
            fprintf(stderr, "Value too large to be stored in data type.\n");
            break;
        case EPERM:
            fprintf(stderr, "Operation not permitted.\n");
            break;
        case EPFNOSUPPORT:
            fprintf(stderr, "Protocol family not supported.\n");
            break;
        case EPIPE:
            fprintf(stderr, "Broken pipe.\n");
            break;
        case EPROTO:
            fprintf(stderr, "Protocol error.\n");
            break;
        case EPROTONOSUPPORT:
            fprintf(stderr, " not supported.\n");
            break;
        case EPROTOTYPE:
            fprintf(stderr, "Protocol wrong type for socket.\n");
            break;
        case ERANGE:
            fprintf(stderr, "Result too large.\n");
            break;
        case EREMCHG:
            fprintf(stderr, "Remote address changed.\n");
            break;
        case EREMOTE:
            fprintf(stderr, "Object is remote.\n");
            break;
        case EREMOTEIO:
            fprintf(stderr, "Remote I/O error.\n");
            break;
        case ERESTART:
            fprintf(stderr, "Interrupted system call should be restarted.\n");
            break;
        case EROFS:
            fprintf(stderr, "Read-only filesystem.\n");
            break;
        case ESHUTDOWN:
            fprintf(stderr, "Cannot send after transport endpoint shutdown.\n");
            break;
        case ESPIPE:
            fprintf(stderr, "Invalid seek.\n");
            break;
        case ESOCKTNOSUPPORT:
            fprintf(stderr, " type not supported.\n");
            break;
        case ESRCH:
            fprintf(stderr, "No such process.\n");
            break;
        case ESTALE:
            fprintf(stderr, "Stale file handle.\n");
            break;
        case ESTRPIPE:
            fprintf(stderr, "Streams pipe error.\n");
            break;
        case ETIME:
            fprintf(stderr, "Timer expired. \n");
            break;
        case ETIMEDOUT:
            fprintf(stderr, "Connection timed out.\n");
            break;
        case ETXTBSY:
            fprintf(stderr, "Text file busy.\n");
            break;
        case EUCLEAN:
            fprintf(stderr, "Structure needs cleaning.\n");
            break;
        case EUNATCH:
            fprintf(stderr, "Protocol driver not attached.\n");
            break;
        case EUSERS:
            fprintf(stderr, "Too many users.\n");
            break;
        case EXDEV:
            fprintf(stderr, "Improper link.\n");
            break;
        case EXFULL:
            fprintf(stderr, "Exchange full.\n");
            break;

        default:
            // no error found
            return 0;
    }

    return errsv;
}

