from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps

class yat4tangoRecipe(ConanFile):
    name = "yat4tango"
    version = "1.14.0"
    package_type = "library"
    user = "soleil"

    # Optional metadata
    license = "GPL-2"
    author = "stephane.poirier@synchrotron-soleil.fr"
    url = "https://gitlab.synchrotron-soleil.fr/software-control-system/libraries/yat4tango"
    description = "Yet Another Toolkit Library for Tango"
    topics = ("utility", "control-system")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/**", "include/**"
    
    def requirements(self):
        self.requires("cpptango/9.2.5@soleil/stable", transitive_headers=True, transitive_libs=True)
        self.requires("yat/1.21.1@soleil/stable", transitive_headers=True, transitive_libs=True)

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        major, minor, patch = map(int, self.version.split('.'))
        tc.variables["VERSION"] = self.version
        tc.variables["MAJOR_VERSION"] = major
        tc.variables["MINOR_VERSION"] = minor
        tc.variables["PATCH_VERSION"] = patch
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["yat4tango"]

        if self.settings.os == "Windows":
            self.cpp_info.defines += ["WIN32_LEAN_AND_MEAN"]

        if self.options.shared and self.settings.os == "Windows":
            self.cpp_info.defines += ["YAT4TANGO_DLL"]