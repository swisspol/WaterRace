import AppKit
import MetalRenderer

let app = NSApplication.shared
app.setActivationPolicy(.regular)

let window = NSWindow(
    contentRect: NSRect(x: 0, y: 0, width: 640, height: 480),
    styleMask: [.titled, .closable, .miniaturizable],
    backing: .buffered,
    defer: false
)
window.title = "WaterRace"
window.center()

let metalView = MetalView(frame: window.contentView!.bounds)!
metalView.autoresizingMask = [.width, .height]
window.contentView = metalView

window.makeKeyAndOrderFront(nil)
app.activate(ignoringOtherApps: true)

let mainMenu = NSMenu()
let appMenuItem = NSMenuItem()
mainMenu.addItem(appMenuItem)
let appMenu = NSMenu()
appMenuItem.submenu = appMenu

let fileMenuItem = NSMenuItem(title: "File", action: nil, keyEquivalent: "")
mainMenu.addItem(fileMenuItem)
let fileMenu = NSMenu(title: "File")
fileMenuItem.submenu = fileMenu
fileMenu.addItem(NSMenuItem(title: "Quit", action: #selector(NSApplication.terminate(_:)), keyEquivalent: "q"))

app.mainMenu = mainMenu

class AppDelegate: NSObject, NSApplicationDelegate {
    func applicationShouldTerminateAfterLastWindowClosed(_ app: NSApplication) -> Bool { true }
}

NotificationCenter.default.addObserver(
    forName: NSWindow.willCloseNotification,
    object: window,
    queue: .main
) { _ in NSApp.terminate(nil) }

let delegate = AppDelegate()
app.delegate = delegate
app.run()
