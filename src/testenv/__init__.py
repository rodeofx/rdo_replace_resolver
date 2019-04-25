# pylint: disable=all

# HACK
#
# There is no way from the pxrConfig.cmake file to get library paths of USD
# dependencies like boost python, tbb, etc
# Importing Sdf here ensures that all required symbols are available before
# importing rdo.ReplaceResolver
#
# For some reason import Tf is not enough in Houdini: Sdf needs to be imported

import pxr.Sdf
