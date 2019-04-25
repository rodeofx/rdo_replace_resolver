# Copyright 2019 Rodeo FX.  All rights reserved.

import os
import unittest
import shutil

from pxr import Ar
from pxr import Kind
from pxr import Sdf
from pxr import Usd
from pxr import Vt

from rdo import ReplaceResolver


def _GetRelativePath(dirName, asset, version):
    return os.path.join(dirName, asset, version, "%s.usda" % asset)


def _PrepAssets(rootDir, assemblyRepo, componentRepo):
    """ Construct following structure

    a/v1/a.usda     b/v1/b.usda      c/v1/c.usda
    /a  ---(R)---> /b   ---(R)---> /c
                                     c = "c_v1"                        
                    
                    b/v2/b.usda      c/v2/c.usda
                     b = "b_v2"       c = "c_v2"

    a/v2/a.usda
    / customLayerData = replace string list
    /   ---(S)---> a1_v1.usda

    c and b attributes are used to check if the stage is 
    composed accordingly when our resolver replace versions.
    """

    context = ReplaceResolver.ReplaceResolverContext([os.path.abspath(rootDir)])

    # Create layer "c.usda" version 1
    assetName = "c"
    assetVersion = "v1"
    c_v1_relativePath = _GetRelativePath("component", assetName, assetVersion)
    _path = os.path.join(rootDir, c_v1_relativePath)
    stage = Usd.Stage.CreateNew(_path, context)
    prim = stage.DefinePrim("/c")
    stage.SetDefaultPrim(prim)

    modelAPI = Usd.ModelAPI(prim)
    modelAPI.SetKind(Kind.Tokens.component)
    modelAPI.SetAssetName(assetName)
    modelAPI.SetAssetVersion(assetVersion)
    modelAPI.SetAssetIdentifier(c_v1_relativePath)

    cAttr = prim.CreateAttribute("c", Sdf.ValueTypeNames.String, True)
    cAttr.Set("c_v1")
    stage.Save()

    # Create layer "c.usda" version 2
    assetVersion = "v2"
    c_v2_relativePath = _GetRelativePath("component", assetName, assetVersion)
    _path = os.path.join(rootDir, c_v2_relativePath)
    stage = Usd.Stage.CreateNew(_path, context)
    stage.GetRootLayer().subLayerPaths = [ "../v1/%s.usda" % assetName ]

    prim = stage.GetPrimAtPath("/c")
    stage.SetDefaultPrim(prim)

    modelAPI = Usd.ModelAPI(prim)
    modelAPI.SetAssetName(assetName)
    modelAPI.SetAssetVersion(assetVersion)
    modelAPI.SetAssetIdentifier(c_v2_relativePath)

    cAttr = prim.GetAttribute("c")
    cAttr.Set("c_v2")
    stage.Save()

    # Create layer "b.usda" version 1
    assetName = "b"
    assetVersion = "v1"

    b_v1_relativePath = _GetRelativePath("assembly", assetName, assetVersion)
    _path = os.path.join(rootDir, b_v1_relativePath)
    stage = Usd.Stage.CreateNew(_path, context)
    prim = stage.DefinePrim("/b")
    stage.SetDefaultPrim(prim)

    modelAPI = Usd.ModelAPI(prim)
    modelAPI.SetKind(Kind.Tokens.assembly)

    prim.GetReferences().AddReference(c_v1_relativePath)
    stage.Save()

    # Create layer "b.usda" version 2
    assetVersion = "v2"

    b_v2_relativePath = _GetRelativePath("assembly", assetName, assetVersion)
    _path = os.path.join(rootDir, b_v2_relativePath)
    stage = Usd.Stage.CreateNew(_path, context)
    stage.GetRootLayer().subLayerPaths = [ "../v1/%s.usda" % assetName ]
    prim = stage.GetPrimAtPath("/b")
    modelAPI = Usd.ModelAPI(prim)
    modelAPI.SetKind(Kind.Tokens.assembly)

    stage.SetDefaultPrim(prim)
    bAttr = prim.CreateAttribute("b", Sdf.ValueTypeNames.String, True)
    bAttr.Set("b_v2")

    stage.Save()

    # Create layer "a.usda" version 1
    assetName = "a"
    assetVersion = "v1"

    a_v1_relativePath = _GetRelativePath("assembly", assetName, assetVersion)
    _path = os.path.join(rootDir, a_v1_relativePath)
    stage = Usd.Stage.CreateNew(_path, context)
    prim = stage.DefinePrim("/a")
    stage.SetDefaultPrim(prim)

    modelAPI = Usd.ModelAPI(prim)
    modelAPI.SetKind(Kind.Tokens.assembly)

    prim.GetReferences().AddReference(b_v1_relativePath)
    stage.Save()

    # Create layer "a.usda" version 2 
    # with customLayerData to store replace strings
    assetVersion = "v2"

    a_v2_relativePath = _GetRelativePath("assembly", assetName, assetVersion)
    _path = os.path.join(rootDir, a_v2_relativePath)
    stage = Usd.Stage.CreateNew(_path, context)
    pair1 = ["component/c/v1/c.usda", "component/c/v2/c.usda"]
    pair2 = ["assembly/b/v1/b.usda", "assembly/b/v2/b.usda"]
    stage.SetMetadata(
        "customLayerData",
        {ReplaceResolver.Tokens.replacePairs: Vt.StringArray(pair1 + pair2)},
    )
    stage.GetRootLayer().subLayerPaths = [ "../v1/%s.usda" % assetName ]
    stage.Save()


class TestReplaceResolver(unittest.TestCase):

    rootDir = "testReplaceResolver"
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
        Ar.SetPreferredResolver("ReplaceResolver")

        # Verify that the underlying resolver is a ReplaceResolver.
        assert isinstance(Ar.GetUnderlyingResolver(), ReplaceResolver.ReplaceResolver)

        # Cleanup files from previous test
        dirToRemove = os.path.abspath(TestReplaceResolver.rootDir)
        if os.path.isdir(dirToRemove):
            shutil.rmtree(dirToRemove)

        _PrepAssets(
            TestReplaceResolver.rootDir,
            TestReplaceResolver.assemblyRepo,
            TestReplaceResolver.componentRepo,
        )

    def test_ResolveWithContext(self):
        context = ReplaceResolver.ReplaceResolverContext(
            [os.path.abspath(TestReplaceResolver.rootDir)]
        )

        context.AddReplacePair("component/c/v1/c.usda", "component/c/v2/c.usda")
        context.AddReplacePair("assembly/b/v1/b.usda", "assembly/b/v2/b.usda")

        with Ar.ResolverContextBinder(context):
            resolver = Ar.GetResolver()

            self.assertPathsEqual(
                resolver.Resolve("component/c/v1/c.usda"),
                os.path.abspath(os.path.join(
                    TestReplaceResolver.rootDir, "component/c/v2/c.usda"))
            )

            self.assertPathsEqual(
                resolver.Resolve("assembly/b/v1/b.usda"), 
                os.path.abspath(os.path.join(
                    TestReplaceResolver.rootDir, "assembly/b/v2/b.usda"))
            )

    def test_ResolveFromStageOneLevel(self):
        """ Replace reference to c/v1 by c/v2 and open stage to check x value """
        context = ReplaceResolver.ReplaceResolverContext(
            [os.path.abspath(TestReplaceResolver.rootDir)]
        )

        context.AddReplacePair("component/c/v1/c.usda", "component/c/v2/c.usda")

        filePath = os.path.join(TestReplaceResolver.assemblyRepo, "b", "v1", "b.usda")
        stage = Usd.Stage.Open(filePath, pathResolverContext=context)

        # "/b" originaly reference "c/v1" that should be replaced by "c/v2"
        prim = stage.GetPrimAtPath("/b")
        cAttr = prim.GetAttribute("c")

        self.assertEqual(cAttr.Get(), "c_v2")

        # Check that asset name, version and path are matching c/v2
        modelAPI = Usd.ModelAPI(prim)
        self.assertEqual(modelAPI.GetKind(), Kind.Tokens.assembly)
        self.assertEqual(modelAPI.GetAssetName(), "c")
        self.assertEqual(modelAPI.GetAssetVersion(), "v2")
        self.assertEqual(modelAPI.GetAssetIdentifier().path, "component/c/v2/c.usda")

    def test_ResolveFromStageTwoLevels(self):
        """ 
        In a/v1, replace reference to b/v1 by b/v2
        In b/v2, replace reference to c/v1 by c/v2

        Check x and y values
        """
        context = ReplaceResolver.ReplaceResolverContext(
            [os.path.abspath(TestReplaceResolver.rootDir)]
        )
        context.AddReplacePair("component/c/v1/c.usda", "component/c/v2/c.usda")
        context.AddReplacePair("assembly/b/v1/b.usda", "assembly/b/v2/b.usda")

        filePath = os.path.join(TestReplaceResolver.assemblyRepo, "a", "v1", "a.usda")
        stage = Usd.Stage.Open(filePath, pathResolverContext=context)

        # "/a" originaly reference "c/v1" that should be replaced by "c/v2"
        prim = stage.GetPrimAtPath("/a")
        bAttr = prim.GetAttribute("b")

        # This value exists only in "b/v2" layer ("b/v2/b.usda" sublayer "b/v1/b.usda")
        self.assertEqual(bAttr.Get(), "b_v2")

        # Check that asset name, version and path are matching c/v2
        modelAPI = Usd.ModelAPI(prim)
        self.assertEqual(modelAPI.GetKind(), Kind.Tokens.assembly)
        self.assertEqual(modelAPI.GetAssetName(), "c")
        self.assertEqual(modelAPI.GetAssetVersion(), "v2")
        self.assertEqual(modelAPI.GetAssetIdentifier().path, "component/c/v2/c.usda")

    def test_ReplaceFromUsdFile(self):
        """ 
        Open a layer with some replace pairs in customLayerData
        In a/v2, replace reference to b/v1 by b/v2
        In b/v2, replace reference to c/v1 by c/v2

        Check x and y values
        """
        filePath = os.path.join(TestReplaceResolver.assemblyRepo, "a", "v2", "a.usda")
        os.environ["PXR_AR_DEFAULT_SEARCH_PATH"] = os.path.abspath(
            TestReplaceResolver.rootDir
        )
        stage = Usd.Stage.Open(filePath)

        # "a/v2/a.usda" sublayer "a/v1/a.usda" that is defining "/a"
        prim = stage.GetPrimAtPath("/a")
        self.assertTrue(prim)

        # "/a" originaly reference "c/v1" that should be replaced by "c/v2"
        cAttr = prim.GetAttribute("c")
        self.assertTrue(cAttr)
        self.assertEqual(cAttr.Get(), "c_v2")

        # This value exists only in "b/v2" layer ("b/v2" sublayer "b/v1")
        bAttr = prim.GetAttribute("b")
        self.assertTrue(bAttr)
        self.assertEqual(bAttr.Get(), "b_v2")

        # Check that asset name, version and path are matching c/v2
        modelAPI = Usd.ModelAPI(prim)
        self.assertEqual(modelAPI.GetKind(), Kind.Tokens.assembly)
        self.assertEqual(modelAPI.GetAssetName(), "c")
        self.assertEqual(modelAPI.GetAssetVersion(), "v2")
        self.assertEqual(modelAPI.GetAssetIdentifier().path, "component/c/v2/c.usda")

    def test_ReplaceFromJsonFile(self):
        """ 
        Open a layer with a side car json file storing the replace pairs
        In a/v2, replace reference to b/v1 by b/v2
        In b/v2, replace reference to c/v1 by c/v2

        Check x and y values
        """
        jsonFilePath = os.path.join(
            TestReplaceResolver.assemblyRepo, "a", "v1", ReplaceResolver.Tokens.replaceFileName
        )

        pair1 = ["component/c/v1/c.usda", "component/c/v2/c.usda"]
        pair2 = ["assembly/b/v1/b.usda", "assembly/b/v2/b.usda"]

        import json

        with open(jsonFilePath, "w") as outfile:
            json.dump([pair1, pair2], outfile)

        filePath = os.path.join(TestReplaceResolver.assemblyRepo, "a/v1/a.usda")
        os.environ["PXR_AR_DEFAULT_SEARCH_PATH"] = os.path.abspath(
            TestReplaceResolver.rootDir
        )
        stage = Usd.Stage.Open(filePath)

        prim = stage.GetPrimAtPath("/a")
        self.assertTrue(prim)

        cAttr = prim.GetAttribute("c")
        self.assertTrue(cAttr)
        self.assertEqual(cAttr.Get(), "c_v2")

        bAttr = prim.GetAttribute("b")
        self.assertTrue(bAttr)
        self.assertEqual(bAttr.Get(), "b_v2")

        # Check that asset name, version and path are matching c/v2
        modelAPI = Usd.ModelAPI(prim)
        self.assertEqual(modelAPI.GetKind(), Kind.Tokens.assembly)
        self.assertEqual(modelAPI.GetAssetName(), "c")
        self.assertEqual(modelAPI.GetAssetVersion(), "v2")
        self.assertEqual(modelAPI.GetAssetIdentifier().path, "component/c/v2/c.usda")


if __name__ == "__main__":
    unittest.main()
