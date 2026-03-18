#import <Cocoa/Cocoa.h>

#include "cocoa_bridge.h"

#include <mutex>
#include <deque>
#include <vector>
#include <cctype>
#include <algorithm>

namespace
{
    std::mutex g_mu;
    std::deque<int> g_events;
    std::vector<uint16_t> g_fb565;
    int g_fb_w = 240;
    int g_fb_h = 240;
    bool g_has_fb = false;

    int g_pixel_w = 240;
    int g_pixel_h = 240;
    double g_scale = 1.0;

    NSWindow *g_window = nil;
    NSView *g_view = nil;
    bool g_should_quit = false;

    uint32_t rgb565_to_rgba8(uint16_t c)
    {
        // Expand 565 -> 888, output RGBA (alpha=255).
        const uint8_t r5 = (uint8_t)((c >> 11) & 0x1F);
        const uint8_t g6 = (uint8_t)((c >> 5) & 0x3F);
        const uint8_t b5 = (uint8_t)((c >> 0) & 0x1F);
        const uint8_t r8 = (uint8_t)((r5 * 255 + 15) / 31);
        const uint8_t g8 = (uint8_t)((g6 * 255 + 31) / 63);
        const uint8_t b8 = (uint8_t)((b5 * 255 + 15) / 31);
        return (uint32_t)r8 | ((uint32_t)g8 << 8) | ((uint32_t)b8 << 16) | 0xFF000000u;
    }
}

@interface FlopperView : NSView
@end

@implementation FlopperView
// Use default (origin bottom-left) and explicitly flip the framebuffer when drawing.
- (BOOL)isFlipped { return NO; }

- (BOOL)acceptsFirstResponder { return YES; }

- (void)keyDown:(NSEvent *)event
{
    NSString *chars = [event charactersIgnoringModifiers];
    if (!chars || [chars length] == 0)
        return;
    unichar ch = [chars characterAtIndex:0];

    int mapped = -1;
    switch (ch)
    {
    case NSUpArrowFunctionKey:
        mapped = 0;
        break;
    case NSDownArrowFunctionKey:
        mapped = 1;
        break;
    case NSLeftArrowFunctionKey:
        mapped = 2;
        break;
    case NSRightArrowFunctionKey:
        mapped = 3;
        break;
    case '\r':
    case '\n':
        mapped = 4;
        break;
    default:
        break;
    }

    if (mapped == -1)
    {
        const unichar lower = (unichar)std::tolower((int)ch);
        if (lower == 'u')
            mapped = 0;
        else if (lower == 'd')
            mapped = 1;
        else if (lower == 'l')
            mapped = 2;
        else if (lower == 'r')
            mapped = 3;
        else if (lower == 'c')
            mapped = 4;
    }

    if (mapped != -1)
    {
        std::lock_guard<std::mutex> lk(g_mu);
        g_events.push_back(mapped);
        return;
    }

    [super keyDown:event];
}

- (void)drawRect:(NSRect)dirtyRect
{
    (void)dirtyRect;
    CGContextRef ctx = [[NSGraphicsContext currentContext] CGContext];
    if (!ctx)
        return;

    std::vector<uint16_t> fb;
    int w = 0;
    int h = 0;
    {
        std::lock_guard<std::mutex> lk(g_mu);
        if (!g_has_fb)
        {
            [[NSColor blackColor] setFill];
            NSRectFill(NSMakeRect(0, 0, g_pixel_w, g_pixel_h));
            return;
        }
        fb = g_fb565;
        w = g_fb_w;
        h = g_fb_h;
    }

    // Convert to RGBA8888.
    std::vector<uint32_t> rgba;
    rgba.resize((size_t)w * (size_t)h);
    for (size_t i = 0; i < rgba.size(); i++)
        rgba[i] = rgb565_to_rgba8(fb[i]);

    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    const size_t bytes_per_row = (size_t)w * 4;
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, rgba.data(), rgba.size() * sizeof(uint32_t), NULL);
    CGImageRef img = CGImageCreate((size_t)w, (size_t)h, 8, 32, bytes_per_row, cs,
                                  (CGBitmapInfo)kCGBitmapByteOrder32Little | (CGBitmapInfo)kCGImageAlphaPremultipliedFirst,
                                  provider, NULL, false, kCGRenderingIntentDefault);

    CGContextSetInterpolationQuality(ctx, kCGInterpolationNone);
    const NSRect b = [self bounds];
    CGContextDrawImage(ctx, CGRectMake(0, 0, b.size.width, b.size.height), img);

    CGImageRelease(img);
    CGDataProviderRelease(provider);
    CGColorSpaceRelease(cs);
}
@end

@interface FlopperWindow : NSWindow
@end

@implementation FlopperWindow
- (void)keyDown:(NSEvent *)event
{
    NSString *chars = [event charactersIgnoringModifiers];
    if (!chars || [chars length] == 0)
        return;
    unichar ch = [chars characterAtIndex:0];

    int mapped = -1;
    switch (ch)
    {
    case NSUpArrowFunctionKey:
        mapped = 0; // UP
        break;
    case NSDownArrowFunctionKey:
        mapped = 1; // DOWN
        break;
    case NSLeftArrowFunctionKey:
        mapped = 2; // LEFT
        break;
    case NSRightArrowFunctionKey:
        mapped = 3; // RIGHT
        break;
    case '\r':
    case '\n':
        mapped = 4; // CENTER
        break;
    default:
        break;
    }

    // Letter shortcuts.
    if (mapped == -1)
    {
        const unichar lower = (unichar)std::tolower((int)ch);
        if (lower == 'u')
            mapped = 0;
        else if (lower == 'd')
            mapped = 1;
        else if (lower == 'l')
            mapped = 2;
        else if (lower == 'r')
            mapped = 3;
        else if (lower == 'c')
            mapped = 4;
    }

    if (mapped != -1)
    {
        std::lock_guard<std::mutex> lk(g_mu);
        g_events.push_back(mapped);
        return;
    }

    [super keyDown:event];
}
@end

namespace flopper::desktop
{
    void init_window(int pixel_w, int pixel_h, double scale)
    {
        g_pixel_w = pixel_w;
        g_pixel_h = pixel_h;
        g_scale = (scale > 0.0) ? scale : 1.0;

        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

        const NSRect rect = NSMakeRect(0, 0, pixel_w * g_scale, pixel_h * g_scale);
        g_window = [[FlopperWindow alloc]
            initWithContentRect:rect
                      styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable)
                        backing:NSBackingStoreBuffered
                          defer:NO];

        [g_window setTitle:@"flopper-one UI (desktop)" ];
        [g_window center];

        g_view = [[FlopperView alloc] initWithFrame:rect];
        [g_window setContentView:g_view];
        [g_window makeKeyAndOrderFront:nil];

        [g_window makeFirstResponder:g_view];

        [NSApp activateIgnoringOtherApps:YES];
    }

    bool pump_events()
    {
        NSEvent *event = nil;
        do
        {
            event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                       untilDate:[NSDate dateWithTimeIntervalSinceNow:0.0]
                                          inMode:NSDefaultRunLoopMode
                                         dequeue:YES];
            if (event)
                [NSApp sendEvent:event];
        } while (event);

        if (g_window && ![g_window isVisible])
            g_should_quit = true;

        return !g_should_quit;
    }

    bool next_input_event(int &evt_out)
    {
        std::lock_guard<std::mutex> lk(g_mu);
        if (g_events.empty())
            return false;
        evt_out = g_events.front();
        g_events.pop_front();
        return true;
    }

    void render_rgb565(const uint16_t *pixels, int w, int h)
    {
        if (!pixels || w <= 0 || h <= 0)
            return;

        {
            std::lock_guard<std::mutex> lk(g_mu);
            g_fb_w = w;
            g_fb_h = h;
            g_fb565.assign(pixels, pixels + ((size_t)w * (size_t)h));
            g_has_fb = true;
        }

        if (g_view)
            [g_view setNeedsDisplay:YES];
    }
}
