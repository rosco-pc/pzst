#include "eserialport.h"
#include <QStringList>

#include <IOKit/IOTypes.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <CoreFoundation/CoreFoundation.h>

//Return 0 if no serial ports found, otherwise non-zero
static int findSerialPorts(io_iterator_t *matchingServices)
{
    int result = 0;
    CFMutableDictionaryRef classesToMatch;

    // Serial devices are instances of class IOSerialBSDClient
    classesToMatch = IOServiceMatching(kIOSerialBSDServiceValue);
    if (classesToMatch != NULL) {
        CFDictionarySetValue(classesToMatch, CFSTR(kIOSerialBSDTypeKey), CFSTR(kIOSerialBSDRS232Type));
        result = (0 == IOServiceGetMatchingServices(kIOMasterPortDefault, classesToMatch, matchingServices));
    }
    return result;
}

QStringList serialPortList(io_iterator_t serialPortIterator)
{
    QStringList result;
    io_object_t modemService;
    char bsdPath[256];

    while ((modemService = IOIteratorNext(serialPortIterator)))
    {
        CFStringRef bsdPathAsCFString;

        bsdPathAsCFString = (CFStringRef)IORegistryEntryCreateCFProperty(modemService, CFSTR(kIODialinDeviceKey), kCFAllocatorDefault, 0);

        if (bsdPathAsCFString) {
            // Convert the path from a CFString to a C (NUL-terminated) string
            if (CFStringGetCString(bsdPathAsCFString, bsdPath, 256, kCFStringEncodingUTF8)) {
                result << bsdPath;
            }
            CFRelease(bsdPathAsCFString);
        }
        // Release the io_service_t now that we are done with it.

        IOObjectRelease(modemService);
    }
    return result;
}

using namespace PZST;

QStringList ESerialPort::enumeratePorts()
{
    QStringList ports;
    io_iterator_t serialPortIterator;
    if (findSerialPorts(&serialPortIterator)) {
        ports = serialPortList(serialPortIterator);
        IOObjectRelease(serialPortIterator);	// Release the iterator.
    }

    return ports;
}


