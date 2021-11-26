import os

def binary_always_allow():
    # always binary executable true custom
    # step1
    import distutils.command.build_scripts
    distutils.command.build_scripts.tokenize.detect_encoding = lambda x: ("utf-8", [])

    # step2
    import setuptools.command.easy_install

    def install_script(self, dist, script_name, script_text, dev_path=None):
        """Generate a legacy script wrapper and install it"""
        spec = str(dist.as_requirement())
        if script_text is None or (isinstance(script_text, bytes) and script_text.startswith(b"MZ")):
            is_script = False
            self.write_script(script_name, script_text, 'b')
            return

        is_script = setuptools.command.easy_install.is_python_script(script_text, script_name)

        if is_script:
            body = self._load_template(dev_path) % locals()
            script_text = setuptools.command.easy_install.ScriptWriter.get_header(script_text) + body
        self.write_script(script_name, script_text.encode("utf8"), 'b')

    setuptools.command.easy_install.easy_install.install_script = install_script

    # step3
    import pkg_resources

    def get_metadata(self, name):
        if not self.egg_info:
            return ""

        try:
            path = self._get_metadata_path(name)
        except AttributeError:
            path = os.path.join(self.egg_info, *name.split("/"))

        value = self._get(path)
        try:
            return value.decode('utf-8')
        except UnicodeDecodeError:
            return value

    pkg_resources.NullProvider.get_metadata = get_metadata

    # step4
    import setuptools.command.develop
    import distutils.util

    def install_egg_scripts(self, dist):
        if dist is not self.dist:
            return setuptools.command.easy_install.install_egg_scripts(self, dist)

        self.install_wrapper_scripts(dist)

        for script_name in self.distribution.scripts or []:
            script_path = os.path.abspath(distutils.util.convert_path(script_name))
            script_name = os.path.basename(script_path)
            try:
                with open(script_path) as strm:
                    script_text = strm.read()
            except UnicodeDecodeError:
                with open(script_path, "rb") as strm:
                    script_text = strm.read()
            self.install_script(dist, script_name, script_text, script_path)

    setuptools.command.develop.develop.install_egg_scripts = install_egg_scripts
