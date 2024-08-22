from conan import ConanFile

class yat4tangoRecipe(ConanFile):
    name = "yat4tango"
    version = "1.14.1"
    package_type = "library"
    user = "soleil"
    python_requires = "base/[>=1.0]@soleil/stable"
    python_requires_extend = "base.Library"

    license = "GPL-2"
    author = "stephane.poirier@synchrotron-soleil.fr"
    url = "https://gitlab.synchrotron-soleil.fr/software-control-system/libraries/yat4tango"
    description = "Yet Another Toolkit Library for Tango"
    topics = ("utility", "control-system")

    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    exports_sources = "CMakeLists.txt", "src/**", "include/**"
    
    def requirements(self):
        self.requires("cpptango/9.2.5@soleil/stable", transitive_headers=True, transitive_libs=True)
        self.requires("yat/[>=1.0]@soleil/stable", transitive_headers=True, transitive_libs=True)

    def package_info(self):
        self.cpp_info.libs = ["yat4tango"]

        if self.settings.os == "Windows":
            self.cpp_info.defines += ["WIN32_LEAN_AND_MEAN"]

        if self.options.shared and self.settings.os == "Windows":
            self.cpp_info.defines += ["YAT4TANGO_DLL"]