if t == nil then
 t = -1;
 lasttime = os.clock()
end
if (os.clock() > lasttime + 0.2) then
  x = math.sin(t) * math.cos(t) * math.log(math.abs(t))
  y = math.sqrt(math.abs(t)) * math.cos(t)
  Horde3D.setNodeTransform(3, x * 10, y * 10, 5, 0, 0, 0, 1, 1, 1)  
  Horde3D.setNodeTransform(4, x * 20, y * 20 - 3.8, 0, 0, 0, 0, 1, 1, 1)
 lasttime = os.clock()
end

 -- camX = math.cos(os.clock() * 0.2) 
 -- camZ = math.sin(os.clock() * 0.2) 

  --camRY = math.acos(camZ) * 180 / 3.141592
  --if (camX < 0) then
  --   camRY = 360 - camRY
  --end
  --Horde3D.setNodeTransform(5, camX * 20, 5, camZ * 20, 0, camRY, 0, 1, 1, 1)

if (t >= 1) then
   t = -1
else
   t = t + 0.01
end