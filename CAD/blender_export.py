import bpy
import math


root_pcb_objs = [
    obj for obj in bpy.data.objects if "PCB" in obj.name and obj.parent is None
]
if not root_pcb_objs:
    raise RuntimeError("No root PCB objects found!")


if len(root_pcb_objs) > 1:
    oldest = root_pcb_objs[0]

    for child in oldest.children:
        bpy.data.objects.remove(child, do_unlink=True)

    bpy.data.objects.remove(oldest, do_unlink=True)

newest = root_pcb_objs[-1]
bpy.ops.object.select_all(action="DESELECT")
newest.select_set(True)
bpy.context.view_layer.objects.active = newest

newest.scale = [100 for s in newest.scale]


newest.location = (-2.08428, 3.8516, 2.37241)


cam = bpy.context.scene.camera
if cam is None:
    raise RuntimeError("No active camera in scene!")

cam.location = (11.4325, 10.9409, 21.5336)
cam.rotation_euler = [math.radians(a) for a in (34.8, -0.000081, 137.2)]


bpy.context.scene.render.engine = "CYCLES"
bpy.context.scene.render.resolution_x = 1920
bpy.context.scene.render.resolution_y = 1080
bpy.context.scene.render.resolution_percentage = 100
bpy.context.scene.render.filepath = "//br1.png"

bpy.ops.render.render(write_still=True)
print("Done!")
