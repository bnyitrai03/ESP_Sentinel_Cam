# -*- coding: utf-8 -*-
#
# English Language RTD & Sphinx config file
#
# Uses ../conf_common.py for most non-language-specific settings.

# Importing conf_common adds all the non-language-specific
# parts to this conf module

try:
    from conf_common import *  # noqa: F403,F401
except ImportError:
    import os
    import sys
    sys.path.insert(0, os.path.abspath('../'))
    from conf_common import *  # noqa: F403,F401

# General information about the project.
project = u'ESP SENTINEL CAM Documentation'
copyright = u'2025, AITIA International, Inc'

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
language = 'en'
