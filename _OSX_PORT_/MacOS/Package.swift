// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "MacOS",
    platforms: [.macOS(.v13)],
    targets: [
        .target(
            name: "MetalRenderer",
            path: "Sources/MetalRenderer",
            publicHeadersPath: "include",
            linkerSettings: [
                .linkedFramework("Metal"),
                .linkedFramework("QuartzCore"),
                .linkedFramework("CoreVideo"),
                .linkedFramework("ImageIO"),
                .linkedFramework("AppKit"),
            ]
        ),
        .executableTarget(
            name: "MacOS",
            dependencies: ["MetalRenderer"],
            path: "Sources/MacOS"
        ),
    ]
)
