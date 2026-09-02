# -*- coding: utf-8 -*-
"""
Build the Hyperbolic demo assets in the editor:
  /Game/Hyperbolic/MPC_Hyperbolic  (Material Parameter Collection)
  /Game/Hyperbolic/M_Hyperbolic    (Material with Klein->Poincare WPO)

Run from the Unreal editor Python console, e.g. via the MCP python tool or:
    py "D:/GameDev/NonEuclidean3DDemo/Content/Hyperbolic/setup_hyperbolic_assets.py"
"""
import unreal

ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
MAT_EDIT = unreal.MaterialEditingLibrary

MPC_PATH = "/Game/Hyperbolic/MPC_Hyperbolic"
MAT_PATH = "/Game/Hyperbolic/M_Hyperbolic"

CUSTOM_CODE = """float3 k = WorldPos / BallRadius;
float kk = dot(k, k);
if (kk >= 0.999f) { return float3(0.0f, 0.0f, 0.0f); }
float s = rsqrt(max(1.0f - kk, 1e-5f));
float4 h = float4(k * s, s);
float4 hl;
hl.x = dot(M0, h);
hl.y = dot(M1, h);
hl.z = dot(M2, h);
hl.w = dot(M3, h);
float3 poincare = hl.xyz / (1.0f + hl.w);
return poincare * BallRadius - WorldPos;"""


def ensure_dir(path):
    unreal.EditorAssetLibrary.make_directory(path)


def create_mpc():
    ensure_dir("/Game/Hyperbolic")
    if unreal.EditorAssetLibrary.does_asset_exist(MPC_PATH):
        mpc = unreal.EditorAssetLibrary.load_asset(MPC_PATH)
        unreal.log("MPC already exists, reusing.")
    else:
        factory = unreal.MaterialParameterCollectionFactoryNew()
        mpc = ASSET_TOOLS.create_asset("MPC_Hyperbolic", "/Game/Hyperbolic", unreal.MaterialParameterCollection, factory)

    # Identity Lorentz transform as defaults.
    defaults = [
        unreal.LinearColor(1.0, 0.0, 0.0, 0.0),
        unreal.LinearColor(0.0, 1.0, 0.0, 0.0),
        unreal.LinearColor(0.0, 0.0, 1.0, 0.0),
        unreal.LinearColor(0.0, 0.0, 0.0, 1.0),
    ]
    params = []
    for i in range(4):
        vec = unreal.CollectionVectorParameter()
        vec.set_editor_property("parameter_name", "HLorentzRow%d" % i)
        vec.set_editor_property("default_value", defaults[i])
        params.append(vec)
    mpc.set_editor_property("vector_parameters", params)
    unreal.EditorAssetLibrary.save_asset(MPC_PATH)
    return mpc


def create_material(mpc):
    ensure_dir("/Game/Hyperbolic")
    if unreal.EditorAssetLibrary.does_asset_exist(MAT_PATH):
        mat = unreal.EditorAssetLibrary.load_asset(MAT_PATH)
        MAT_EDIT.delete_all_material_expressions(mat)
    else:
        factory = unreal.MaterialFactoryNew()
        mat = ASSET_TOOLS.create_asset("M_Hyperbolic", "/Game/Hyperbolic", unreal.Material, factory)

    mat.set_editor_property("two_sided", True)

    # Custom node.
    custom = MAT_EDIT.create_material_expression(mat, unreal.MaterialExpressionCustom, -600, 0)
    custom.set_editor_property("code", CUSTOM_CODE)
    custom.set_editor_property("output_type", unreal.CustomMaterialOutputType.CMOT_Float3)

    inputs = []
    for name in ["WorldPos", "M0", "M1", "M2", "M3", "BallRadius"]:
        ci = unreal.CustomInput()
        ci.set_editor_property("input_name", name)
        inputs.append(ci)
    custom.set_editor_property("inputs", inputs)

    # Absolute World Position -> WorldPos.
    wp = MAT_EDIT.create_material_expression(mat, unreal.MaterialExpressionWorldPosition, -900, 200)
    MAT_EDIT.connect_material_expressions(wp, "", custom, "WorldPos")

    # 4x Collection Parameter -> M0..M3.
    for i in range(4):
        cp = MAT_EDIT.create_material_expression(mat, unreal.MaterialExpressionCollectionParameter, -900, 300 + i * 120)
        cp.set_editor_property("collection", mpc)
        cp.set_editor_property("parameter_name", "HLorentzRow%d" % i)
        MAT_EDIT.connect_material_expressions(cp, "", custom, "M%d" % i)

    # Constant -> BallRadius (world-space Klein ball radius in cm).
    const = MAT_EDIT.create_material_expression(mat, unreal.MaterialExpressionConstant, -900, 800)
    const.set_editor_property("r", 100.0)
    MAT_EDIT.connect_material_expressions(const, "", custom, "BallRadius")

    # Custom output -> World Position Offset.
    MAT_EDIT.connect_material_property(custom, "", unreal.MaterialProperty.MP_WORLD_POSITION_OFFSET)

    # Vertex Color -> Base Color (shows the lattice gradient).
    vc = MAT_EDIT.create_material_expression(mat, unreal.MaterialExpressionVertexColor, -600, 400)
    MAT_EDIT.connect_material_property(vc, "", unreal.MaterialProperty.MP_BASE_COLOR)

    MAT_EDIT.layout_material_expressions(mat)
    errors = MAT_EDIT.recompile_material(mat)
    if errors:
        unreal.log_error("Material compile errors: %s" % errors)
    else:
        unreal.log("Material compiled cleanly.")

    unreal.EditorAssetLibrary.save_asset(MAT_PATH)
    return mat


def main():
    try:
        mpc = create_mpc()
        mat = create_material(mpc)
        unreal.log("Done. MPC=%s Material=%s" % (mpc.get_path_name(), mat.get_path_name()))
    except Exception as e:
        unreal.log_error("setup_hyperbolic_assets failed: %s" % repr(e))


if __name__ == "__main__":
    main()
