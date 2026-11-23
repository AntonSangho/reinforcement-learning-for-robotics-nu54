# nu54/FreeCAD/scripts/build_nu54_model.py
"""
Build a simplified FreeCAD model of the NU54 self-balancing robot from
the hand measurements in docs/hardware.md section 2-1 (2026-11-23).

Creates three objects, labelled like the Bala2 tutorial model so that
mesh_export.py and inertia_utils.py work unchanged:

    chassis      everything except the wheels, one solid
    wheel_left   solid cylinder
    wheel_right  solid cylinder

Frame (same convention as bala2-fire-simplified.FCStd):
    origin = midpoint between the two wheel axle centres
    +X = robot's left, -Y = robot's front, +Z = up, units mm
    Exporting with rpy_degrees=(0, 0, 90) turns this into the MuJoCo frame
    (+X front, +Y left, +Z up).

Each wheel is built around its own origin and moved into place with its
Placement, so mesh export (which strips the Placement) puts every mesh on
its own origin, as in the tutorial.

Run headless (saves workspace/nu54/FreeCAD/nu54/nu54.FCStd):
    freecadcmd workspace/nu54/FreeCAD/scripts/build_nu54_model.py

Or in FreeCAD's Python console:
    exec(open('/path/to/workspace/nu54/FreeCAD/scripts/build_nu54_model.py').read())
"""

from pathlib import Path
import FreeCAD as App
import Part

DOC_NAME = 'nu54'
OUT_PATH = (Path(__file__).resolve().parents[1] / DOC_NAME / f'{DOC_NAME}.FCStd')

# ---------------------------------------------------------------------------
# Measurements (mm). Heights are relative to the axle centre, which sits
# 37.5 mm (wheel radius) above the floor.

AXLE_HEIGHT = 37.5          # floor -> axle centre (wheel radius)

WHEEL_DIAMETER = 75.0
WHEEL_WIDTH = 40.0
TRACK_WIDTH = 245.0         # wheel centre to wheel centre

PLATE = dict(w=195.0, d=98.0, h=5.0, z0=60.0 - AXLE_HEIGHT)     # 22.5
POST = dict(w=17.0, d=70.0, h=105.0, gap=95.0)                  # inner gap
TOP = dict(w=132.0, d=87.0, h=19.0)
BATTERY = dict(w=65.0, d=110.0, h=18.0, z0=40.0 - AXLE_HEIGHT)  # 2.5

# JGA25-370: diameter 25, body length ~60 (gearbox L 21 for 1:34 +
# motor 30.8 + encoder). Placed from the plate edge inward.
MOTOR_DIAMETER = 25.0
MOTOR_LENGTH = 60.0
MOTOR_OUTER_X = PLATE['w'] / 2  # 97.5

# Small connecting pieces so the chassis is one solid (FreeCAD only gives
# centre of mass and inertia for a single solid, not for a compound).
# Real robot: L brackets hold the motors, the battery is strapped/taped.
BRACKET = dict(w=20.0, d=20.0)

# IMU chip centre, MuJoCo frame (front, left, up) in mm. Not part of the
# geometry; printed for the MJCF imu_site.
IMU_POS_MUJOCO = (45.0, 4.5, 75.0 - AXLE_HEIGHT)


def box(x0, x1, y0, y1, z0, z1):
    """Axis-aligned box from min/max corners."""
    return Part.makeBox(x1 - x0, y1 - y0, z1 - z0, App.Vector(x0, y0, z0))


def centred_box(w, d, z0, h, x=0.0, y=0.0):
    """Box centred on (x, y) in plan, from z0 up by h."""
    return box(x - w / 2, x + w / 2, y - d / 2, y + d / 2, z0, z0 + h)


def x_cylinder(radius, x0, x1):
    """Cylinder along X from x0 to x1, centred on the Y=Z=0 axle line."""
    return Part.makeCylinder(radius, x1 - x0, App.Vector(x0, 0, 0),
                             App.Vector(1, 0, 0))


def build_chassis():
    plate_top = PLATE['z0'] + PLATE['h']
    post_top = plate_top + POST['h']

    parts = [centred_box(PLATE['w'], PLATE['d'], PLATE['z0'], PLATE['h'])]

    post_x = POST['gap'] / 2 + POST['w'] / 2
    for side in (+1, -1):
        parts.append(centred_box(POST['w'], POST['d'], plate_top, POST['h'],
                                 x=side * post_x))

    parts.append(centred_box(TOP['w'], TOP['d'], post_top, TOP['h']))

    parts.append(centred_box(BATTERY['w'], BATTERY['d'], BATTERY['z0'],
                             BATTERY['h']))
    # Spacer between battery top and plate bottom (2 mm gap in the measurement)
    batt_top = BATTERY['z0'] + BATTERY['h']
    if PLATE['z0'] > batt_top:
        parts.append(centred_box(BATTERY['w'] - 10, BATTERY['d'] - 10,
                                 batt_top, PLATE['z0'] - batt_top))

    r = MOTOR_DIAMETER / 2
    for side in (+1, -1):
        x_out = side * MOTOR_OUTER_X
        x_in = side * (MOTOR_OUTER_X - MOTOR_LENGTH)
        parts.append(x_cylinder(r, min(x_in, x_out), max(x_in, x_out)))
        # Bracket from inside the motor up to the plate
        bx = side * (MOTOR_OUTER_X - BRACKET['w'] / 2 - 5)
        parts.append(centred_box(BRACKET['w'], BRACKET['d'], r / 2,
                                 PLATE['z0'] - r / 2, x=bx))

    shape = parts[0].multiFuse(parts[1:]).removeSplitter()
    if shape.ShapeType != 'Solid':
        if len(shape.Solids) != 1:
            raise RuntimeError(
                f'chassis is {len(shape.Solids)} separate solids; '
                'pieces must touch so inertia can be computed')
        shape = shape.Solids[0]
    return shape


def build_wheel():
    """Wheel on its own origin, axis along X."""
    return x_cylinder(WHEEL_DIAMETER / 2, -WHEEL_WIDTH / 2, WHEEL_WIDTH / 2)


def build(doc=None):
    if doc is None:
        doc = App.newDocument(DOC_NAME)

    chassis = doc.addObject('Part::Feature', 'chassis')
    chassis.Label = 'chassis'
    chassis.Shape = build_chassis()

    for label, side in (('wheel_left', +1), ('wheel_right', -1)):
        wheel = doc.addObject('Part::Feature', label)
        wheel.Label = label
        wheel.Shape = build_wheel()
        wheel.Placement = App.Placement(
            App.Vector(side * TRACK_WIDTH / 2, 0, 0), App.Rotation())

    doc.recompute()
    return doc


def report(doc):
    for obj in doc.Objects:
        bb = obj.Shape.BoundBox
        print(f'{obj.Label:12s} vol {obj.Shape.Volume / 1e3:8.1f} cm^3  '
              f'X {bb.XMin:7.1f}..{bb.XMax:7.1f}  '
              f'Y {bb.YMin:7.1f}..{bb.YMax:7.1f}  '
              f'Z {bb.ZMin:7.1f}..{bb.ZMax:7.1f}')
    com = doc.getObjectsByLabel('chassis')[0].Shape.CenterOfMass
    print(f'chassis geometric centre (uniform density, FreeCAD frame): '
          f'({com.x:.1f}, {com.y:.1f}, {com.z:.1f}) mm')
    print(f'IMU site, MuJoCo frame (front, left, up): {IMU_POS_MUJOCO} mm')
    print(f'chassis body pos z in MJCF = wheel radius = '
          f'{WHEEL_DIAMETER / 2 / 1000:.4f} m')


# freecadcmd runs a script with __name__ set to the file's stem, not
# '__main__'; the GUI console exec() uses '__main__'.
if __name__ in ('__main__', 'build_nu54_model'):
    _doc = build()
    report(_doc)
    OUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    _doc.saveAs(str(OUT_PATH))
    print(f'saved {OUT_PATH}')
