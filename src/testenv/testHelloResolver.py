import os
from pxr import Ar
from pxr import Usd
from pxr import Kind
from pxr import Sdf

from rdo import HelloResolver

import unittest
import shutil


def _PrepAssets(assemblyRepo, componentRepo):
    ''' Construct the following usd files:
    
    assembly_v1.usda            component_v1.usda
        /ASSEMBLY  -----(R)----->   def "COMPONENT"
                                                    
                                component_v2.usda
                                    subLayers = "component_v1.usda"
                                    over "COMPONENT"
    '''

    # Create component_v1.usda
    compRelativePath = 'component_v1.usda'
    fooPath = os.path.join(componentRepo, compRelativePath)
    stage = Usd.Stage.CreateNew(fooPath)
    prim = stage.DefinePrim('/COMPONENT')
    Usd.ModelAPI(prim).SetKind(Kind.Tokens.component)

    stage.SetDefaultPrim(prim)
    radiusAttr = prim.CreateAttribute("radius", Sdf.ValueTypeNames.Double, True)
    radiusAttr.Set(1.0)
    stage.Save()

    # Create component_v2.usda
    fooRelativePath2 = 'component_v2.usda'
    fooPath2 = os.path.join(componentRepo, fooRelativePath2)
    stage = Usd.Stage.CreateNew(fooPath2)
    stage.GetRootLayer().subLayerPaths = [compRelativePath]

    prim = stage.GetPrimAtPath('/COMPONENT')
    Usd.ModelAPI(prim).SetKind(Kind.Tokens.component)

    stage.SetDefaultPrim(prim)
    radiusAttr = prim.GetAttribute("radius")
    radiusAttr.Set(2.0)
    stage.Save()

    # Create assembly_v1.usda
    assemblyPath = os.path.join(assemblyRepo, 'assembly_v1.usda')
    stage = Usd.Stage.CreateNew(assemblyPath)
    prim = stage.DefinePrim('/ASSEMBLY')
    Usd.ModelAPI(prim).SetKind(Kind.Tokens.assembly)
    stage.SetDefaultPrim(prim)

    prim.GetReferences().AddReference("component/" + compRelativePath)
    stage.Save()


class TestHelloResolver(unittest.TestCase):

    rootDir = 'testHelloResolver'
    componentRepo = os.path.join(rootDir, "component")
    assemblyRepo = os.path.join(rootDir, "assembly")

    def assertPathsEqual(self, path1, path2):
        # Flip backslashes to forward slashes to accommodate platform
        # differences. We don't use os.path.normpath since that might
        # fix up other differences we'd want to catch in these tests.
        self.assertEqual(path1.replace("\\", "/"), path2.replace("\\", "/"))

    @classmethod
    def setUpClass(cls):
        # Force Ar to use the hello resolver implementation.
        Ar.SetPreferredResolver('HelloResolver')

        # Verify that the underlying resolver is a HelloResolver.
        assert(isinstance(Ar.GetUnderlyingResolver(), HelloResolver.HelloResolver))

        # Cleanup files from previous test
        testDir = os.path.abspath(TestHelloResolver.rootDir)
        if os.path.isdir(testDir):
            shutil.rmtree(testDir)

        _PrepAssets(TestHelloResolver.assemblyRepo, TestHelloResolver.componentRepo)

    def test_ResolveWithContext(self):
        context = HelloResolver.HelloResolverContext([os.path.abspath(TestHelloResolver.rootDir)])

        context.ToReplace('component/component_v1.usda', 'component/component_v2.usda')

        with Ar.ResolverContextBinder(context):
            resolver = Ar.GetResolver()

            expectedPath = os.path.join(TestHelloResolver.rootDir, 'component', 'component_v2.usda')
            self.assertEqual(
                resolver.Resolve('component/component_v1.usda'),
                os.path.abspath(expectedPath))

    def test_ResolveFromStage(self):
        ''' Replace reference to component v1 by v2 and open stage to check radius value '''
        context = HelloResolver.HelloResolverContext([os.path.abspath(TestHelloResolver.rootDir)])
        context.ToReplace('component/component_v1.usda', 'component/component_v2.usda')

        filePath = os.path.join(TestHelloResolver.assemblyRepo, 'assembly_v1.usda')
        stage = Usd.Stage.Open(filePath, pathResolverContext=context)

        prim = stage.GetPrimAtPath('/ASSEMBLY')
        radiusAttr = prim.GetAttribute("radius")
        
        self.assertEqual(radiusAttr.Get(), 2.0)


if __name__ == '__main__':
    unittest.main()

