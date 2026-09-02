# -*- coding: utf-8 -*-
"""
Create (or load) the empty hyperbolic demo level and add basic lighting.

Run from the Unreal editor Python console:
    py "D:/GameDev/NonEuclidean3DDemo/Content/Hyperbolic/setup_level.py"
"""
import unreal

LEVEL_PATH = "/Game/Hyperbolic/HyperbolicDemo"


def main():
    if unreal.EditorAssetLibrary.does_asset_exist(LEVEL_PATH):
        unreal.log("Level already exists, loading: %s" % LEVEL_PATH)
        unreal.EditorLevelLibrary.load_level(LEVEL_PATH)
    else:
        ok = unreal.EditorLevelLibrary.new_level(LEVEL_PATH)
        if not ok:
            unreal.log_error("Failed to create level %s" % LEVEL_PATH)
            return

    unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0.0, 0.0, 300.0),
        unreal.Rotator(-50.0, -30.0, 0.0))

    unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0.0, 0.0, 300.0),
        unreal.Rotator(0.0, 0.0, 0.0))

    unreal.EditorLevelLibrary.save_current_level()
    unreal.log("Hyperbolic demo level ready: %s" % LEVEL_PATH)


if __name__ == "__main__":
    main()
