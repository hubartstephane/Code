<?xml version="1.0" encoding="UTF-8"?>
<tileset version="1.2" tiledversion="1.3.0" name="tileset" tilewidth="32" tileheight="32" tilecount="3" columns="0">
 <grid orientation="orthogonal" width="1" height="1"/>
 <tile id="0" type="WALL">
  <image width="32" height="32" source="../sprites/Brick.png"/>
 </tile>
 <tile id="1" type="PlayerStart">
  <properties>
   <property name="BITMAP_NAME" value="Player"/>
   <property name="PARTICLE_CREATION" type="bool" value="false"/>
  </properties>
  <image width="32" height="32" source="../sprites/player.png"/>
 </tile>
 <tile id="2" type="Spike">
  <image width="32" height="32" source="../sprites/spike.png"/>
 </tile>
</tileset>
