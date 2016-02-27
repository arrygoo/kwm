#include "notifications.h"
#include "window.h"
#include "border.h"

extern kwm_screen KWMScreen;
extern kwm_toggles KWMToggles;
extern kwm_tiling KWMTiling;
extern kwm_focus KWMFocus;
extern kwm_mode KWMMode;

void FocusedAXObserverCallback(AXObserverRef Observer, AXUIElementRef Element, CFStringRef Notification, void *ContextData)
{
    Assert(Element, "AXOBserverCallback() Element was null")

    window_info *Window = KWMFocus.Window;
    if(!Window)
        return;

    if(CFEqual(Notification, kAXTitleChangedNotification))
        Window->Name = GetWindowTitle(Element);
    else if(CFEqual(Notification, kAXWindowResizedNotification) ||
            CFEqual(Notification, kAXWindowMovedNotification))
        UpdateBorder("focused");
    else if(CFEqual(Notification, kAXFocusedWindowChangedNotification))
        SetWindowRefFocus(Element);
}

void DestroyApplicationNotifications()
{
    if(!KWMFocus.Observer)
        return;

    AXObserverRemoveNotification(KWMFocus.Observer, KWMFocus.Application, kAXWindowMiniaturizedNotification);
    AXObserverRemoveNotification(KWMFocus.Observer, KWMFocus.Application, kAXWindowMovedNotification);
    AXObserverRemoveNotification(KWMFocus.Observer, KWMFocus.Application, kAXWindowResizedNotification);
    AXObserverRemoveNotification(KWMFocus.Observer, KWMFocus.Application, kAXTitleChangedNotification);
    AXObserverRemoveNotification(KWMFocus.Observer, KWMFocus.Application, kAXFocusedWindowChangedNotification);
    CFRunLoopRemoveSource(CFRunLoopGetCurrent(), AXObserverGetRunLoopSource(KWMFocus.Observer), kCFRunLoopDefaultMode);

    CFRelease(KWMFocus.Observer);
    KWMFocus.Observer = NULL;
    CFRelease(KWMFocus.Application);
    KWMFocus.Application = NULL;
}

void CreateApplicationNotifications()
{
    DestroyApplicationNotifications();

    if(KWMFocus.Window)
    {
        KWMFocus.Application = AXUIElementCreateApplication(KWMFocus.Window->PID);
        if(!KWMFocus.Application)
            return;

        AXError Error = AXObserverCreate(KWMFocus.Window->PID, FocusedAXObserverCallback, &KWMFocus.Observer);
        if(Error == kAXErrorSuccess)
        {
            DEBUG("CREATE NOTIFICATIONS")
            AXObserverAddNotification(KWMFocus.Observer, KWMFocus.Application, kAXWindowMiniaturizedNotification, NULL);
            AXObserverAddNotification(KWMFocus.Observer, KWMFocus.Application, kAXWindowMovedNotification, NULL);
            AXObserverAddNotification(KWMFocus.Observer, KWMFocus.Application, kAXWindowResizedNotification, NULL);
            AXObserverAddNotification(KWMFocus.Observer, KWMFocus.Application, kAXTitleChangedNotification, NULL);
            AXObserverAddNotification(KWMFocus.Observer, KWMFocus.Application, kAXFocusedWindowChangedNotification, NULL);
            CFRunLoopAddSource(CFRunLoopGetCurrent(), AXObserverGetRunLoopSource(KWMFocus.Observer), kCFRunLoopDefaultMode);
        }
    }
}
