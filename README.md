# Replace Resolver,

An asset path resolver plugin for USD.

It is following the ArDefaultResolver logic, such as relative paths resolution using an "anchor"
paths (see [USD API documentation for more info](http://graphics.pixar.com/usd/docs/api/class_ar_default_resolver.html#details))
and is adding a substring replacement system.


## Purpose of the Substring Replacement System

Custom asset resolvers are used to translate any _asset paths_ found in the layer stack of a USD stage.
For example, such _asset path_ can be a URI used to retrieve a specific version of the asset from a data base.

To avoid adding a dependency with a database API, a solution is to access such database before opening a USD file,
and then store the information in the USD layer metadata or in a sidecare file.

The advantage are multiple:
- can resolve the shot from a cloud rendering system not connected to studio database.
- allow sharing of USD files between studios including asset versions overrides (assuming the resolver plugin is sent to other studios)
- allow _pinning_ of versions for specific shots (to garenty that every rendered frames will use the same assets)

## Scenario


Given a stage with some prims referencing some assets as bellow:

- Content of a USD file located in `/myshow/published/shots/foo`
```
#usda 1.0

def "foo_01" (
	prepend references = @assets/foo/v1/foo.usda@
)
{
}

def "bar_01" (
	prepend references = @assets/bar/v4/bar.usda@)
{
}
```
We want to load the stage with a version *v2* of foo and a version *v5* of bar without authoring USD arcs explicitly.


### Different strategies can be used

#### Describing the assets paths to be replaced using the _AddReplacePair_ method of the _ReplaceResolverContext_

Can be useful to quickly test a replace pair for a specific asset path.

Adantage: 
Allow to open a USD file without any "replace versions" information in its meta data or sidecar json file).

Inconvenient:
Code for the stage loaders in the DCC plugins or usdview must implement such logic.

```
# Python script to be executed from `/myshow`
from pxr import Ar
from pxr import Usd
from rdo import ReplaceResolver

import os

# Add an anchor path in the initialization of the ReplaceResolverContext
# to find our assets since they are referenced using relative paths
context = ReplaceResolver.ReplaceResolverContext(
	[os.path.join( os.getcwd(), 'published')])

# To replace foo v1 by foo v2
context.AddReplacePair('assets/foo/v1/foo.usda', 'assets/foo/v2/foo.usda')
# To replace bar v4 by bar v5
context.AddReplacePair('assets/bar/v4/bar.usda', 'assets/bar/v5/bar.usda')

# Open the stage with our context
stage = Usd.Stage.Open('published/shots/a_v1.usda', context)

# assuming foo and bar prims have a string attribute "version" matching their file version
assert (stage.GetPrimAtPath('/foo_01').GetAttribute('version').Get() == "v2")
assert (stage.GetPrimAtPath('/bar_01').GetAttribute('version').Get() == "v5")
``` 

### Adding replace information in the 'customLayerData' of the root layer

This is the recommanded way in production.


```
# Python script executed from `/myshow`
from pxr import Ar
from pxr import Usd
from pxr import Vt
from rdo import ReplaceResolver
from shutil import copyfile

import os

# Copy the shot 'a_v1' we want to override to 'a_v2'
# (alternatively we could just "Stage.CreateNew('a_v2.usda')" and sublayer 'a_v1.usda'
copyfile('published/shots/a_v1.usda', 'published/shots/a_v2.usda')

stage = Usd.Stage.Open('published/shots/a_v2.usda')
replaceFoo = ['assets/foo/v1/foo.usda', 'assets/foo/v2/foo.usda']
replacerBar = ['assets/bar/v4/bar.usda', 'assets/bar/v5/bar.usda']

# We need to use a Vt array to set the data properly from Python
# https://github.com/PixarAnimationStudios/USD/issues/813
vtStringArray = Vt.StringArray(replaceFoo + replacerBar)

# Add the replace data to the stage custom meta data
stage.SetMetadata('customLayerData', {ReplaceResolver.Tokens.replacePairs: vtStringArray})
stage.Save()

# Setup our anchor search path using the environmnet variable
os.environ['PXR_AR_DEFAULT_SEARCH_PATH'] = os.path.abspath('published')

# Open the new shot version and check the version attributes
stage = Usd.Stage.Open('published/shots/a_v2.usda')
assert (stage.GetPrimAtPath('/foo_01').GetAttribute('version').Get() == "v2")
assert (stage.GetPrimAtPath('/bar_01').GetAttribute('version').Get() == "v5")
```

## Using a side car json

If a file called "replace.json" exists in the same directory than the Usd file with the following data,
it will be used to replace substrings. This can be used to override a USD file including.

```
[
	['assets/foo/v1/foo.usda', 'assets/foo/v2/foo.usda'], 
	['assets/bar/v4/bar.usda', 'assets/bar/v5/bar.usda']
]
```

## Debug code

Adding following tokens to *TD_DEBUG* will print ReplaceResolver information
* REPLACERESOLVER_PATH
* REPLACERESOLVER_REPLACE
* REPLACERESOLVER_CURRENTCONTEXT

`export TF_TOKEN=REPLACERESOLVER_PATH `
