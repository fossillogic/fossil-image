# **Fossil Image by Fossil Logic**

**Fossil Image** is a lightweight, portable image processing and manipulation library written in pure C with zero external dependencies. Designed for simplicity and speed, Fossil Image supports image loading, saving, and basic transformations across a wide range of formats. Whether you need to handle images in embedded applications, cross-platform tools, or custom pipelines, Fossil Image delivers reliability with minimal overhead.  

### Key Features

- **Cross-Platform Support**  
  Runs seamlessly across desktop, server, and embedded platforms.  

- **Zero External Dependencies**  
  Built entirely in portable C for a consistent, dependency-free experience.  

- **Image Format Support**  
  Handles popular image formats (e.g., *PNG*, *BMP*, *JPEG*) with a focus on speed and correctness.  

- **Lightweight and Efficient**  
  Optimized for small binaries and low memory usage, perfect for resource-constrained systems.  

- **Self-Contained & Auditable**  
  Straightforward, review-friendly code suitable for safety-critical projects.  

- **Modular Design**  
  Provides core image utilities while allowing easy extension for advanced processing needs.

## ***Prerequisites***

To get started, ensure you have the following installed:

- **Meson Build System**: If you don’t have Meson `1.8.0` or newer installed, follow the installation instructions on the official [Meson website](https://mesonbuild.com/Getting-meson.html).

### Adding Dependency

#### Adding via Meson Git Wrap

To add a git-wrap, place a `.wrap` file in `subprojects` with the Git repo URL and revision, then use `dependency('fossil-image')` in `meson.build` so Meson can fetch and build it automatically.

#### Integrate the Dependency:

Add the `fossil-image.wrap` file in your `subprojects` directory and include the following content:

```ini
[wrap-git]
url = https://github.com/fossillogic/fossil-image.git
revision = v0.1.0

[provide]
dependency_names = fossil-image
```

**Note**: For the best experience, always use the latest releases. Visit the [releases](https://github.com/fossillogic/fossil-image/releases) page for the latest versions.

## Build Configuration Options

Customize your build with the following Meson options:
	•	Enable Tests
To run the built-in test suite, configure Meson with:

```sh
meson setup builddir -Dwith_test=enabled
```

### Tests Double as Samples

The project is designed so that **test cases serve two purposes**:

- ✅ **Unit Tests** – validate the framework’s correctness.  
- 📖 **Usage Samples** – demonstrate how to use these libraries through test cases.  

This approach keeps the codebase compact and avoids redundant “hello world” style examples.  
Instead, the same code that proves correctness also teaches usage.  

This mirrors the **Meson build system** itself, which tests its own functionality by using Meson to test Meson.  
In the same way, Fossil Logic validates itself by demonstrating real-world usage in its own tests via Fossil Test.  

```bash
meson test -C builddir -v
```

Running the test suite gives you both verification and practical examples you can learn from.

## Contributing and Support

For those interested in contributing, reporting issues, or seeking support, please open an issue on the project repository or visit the [Fossil Logic Docs](https://fossillogic.com/docs) for more information. Your feedback and contributions are always welcome.
