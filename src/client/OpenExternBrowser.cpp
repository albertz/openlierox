/*
 *  OpenExternBrowser.cpp
 *  OpenLieroX
 *
 *  Created by Albert Zeyer on 29.09.08.
 *  code under LGPL
 *
 */


#include <string>


#ifdef __APPLE__
#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#endif


// declared in AuxLib.h; we are not including this to avoid problems with Rect
void OpenLinkInExternBrowser(const std::string& url) {
#if defined(__APPLE__)
	// Thanks to Jooleem project (http://jooleem.sourceforge.net) for the code

	// Create a string ref of the URL:
	CFStringRef cfurlStr = CFStringCreateWithCString( NULL, url.c_str(), kCFStringEncodingASCII);

	// Create a URL object:
	CFURLRef cfurl = CFURLCreateWithString (NULL, cfurlStr, NULL);

	// Open the URL:
	LSOpenCFURLRef(cfurl, NULL);

	// Release the created resources:
	CFRelease(cfurl);
	CFRelease(cfurlStr);

#elif defined(WIN32)
	ShellExecute(NULL, "open", url.c_str(), NULL, NULL, SW_MAXIMIZE);
	
#else

#endif
}

