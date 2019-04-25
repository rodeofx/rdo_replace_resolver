# pylint: disable=all

# HACK
#
# There is no way from the pxrConfig.cmake file to get library paths of USD
# dependencies like boost python, tbb, etc
# Importing Tf here ensures that all required symbols are available before
# importing rdo.ReplaceResolver

import pxr.Tf
