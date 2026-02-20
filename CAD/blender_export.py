import bpy
import math


root_pcb_objs = [obj for obj in bpy.data.objects if "PCB" in obj.name and obj.parent is None]
if not root_pcb_objs:
    raise RuntimeError("No root PCB objects found!")


selected_root_pcbs = [obj for obj in root_pcb_objs if obj.select_get()]
if not selected_root_pcbs:
    
    newest = max(root_pcb_objs, key=lambda o: o.as_pointer())
else:
    newest = selected_root_pcbs[-1]


for obj in root_pcb_objs:
    if obj != newest:
        for child in obj.children:
            bpy.data.objects.remove(child, do_unlink=True)
        bpy.data.objects.remove(obj, do_unlink=True)


bpy.ops.object.select_all(action="DESELECT")
newest.select_set(True)
bpy.context.view_layer.objects.active = newest


newest.scale = [100 for s in newest.scale]
newest.location = (-2.08428, 3.8516, 2.37241)


cam = bpy.context.scene.camera
cam.location = (11.4325, 10.9409, 21.5336)
cam.rotation_euler = [math.radians(a) for a in (34.8, -0.000081, 137.2)]


bpy.context.scene.render.engine = "CYCLES"
bpy.context.scene.render.resolution_x = 1920
bpy.context.scene.render.resolution_y = 1080
bpy.context.scene.render.resolution_percentage = 100
bpy.context.scene.render.filepath = "//../docs/br1.png"
bpy.ops.render.render(write_still=True)

print("Render complete!")