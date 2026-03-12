import AppKit
import AVFoundation
import MetalRenderer

let app = NSApplication.shared
app.setActivationPolicy(.regular)

let args = CommandLine.arguments.dropFirst()
let isTest = args.contains("-test")
let validModes: Set<String> = ["characters", "boats", "levels"]
let mode: String? = args.first(where: { validModes.contains($0) })

let titles = ["characters": "WaterRace – Characters",
               "boats":      "WaterRace – Boats",
               "levels":     "WaterRace – Levels"]
let windowTitle = mode.flatMap { titles[$0] } ?? "WaterRace – Logo Intro"

let window = NSWindow(
    contentRect: NSRect(x: 0, y: 0, width: 640, height: 480),
    styleMask: [.titled, .closable, .miniaturizable],
    backing: .buffered,
    defer: false
)
window.title = windowTitle
window.center()

let logoView = LogoIntroView(frame: window.contentView!.bounds, mode: mode)!
logoView.autoresizingMask = [.width, .height]
window.contentView = logoView

window.makeKeyAndOrderFront(nil)
window.makeFirstResponder(logoView)
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

var player: AVAudioPlayer?
let musicURL = URL(fileURLWithPath: FileManager.default.currentDirectoryPath)
    .appendingPathComponent("Music.mp3")
if let p = try? AVAudioPlayer(contentsOf: musicURL) {
    p.numberOfLoops = -1
    p.play()
    player = p
}

if isTest {
    DispatchQueue.main.asyncAfter(deadline: .now() + 5) { NSApp.terminate(nil) }
}

let delegate = AppDelegate()
app.delegate = delegate
app.run()
