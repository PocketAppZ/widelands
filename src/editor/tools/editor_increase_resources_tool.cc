/*
 * Copyright (C) 2002-4 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "editor_increase_resources_tool.h"
#include "map.h"
#include "field.h"
#include "editorinteractive.h"
#include "world.h"
#include "map.h"
#include "error.h"
#include "overlay_manager.h"

/*
=============================

class Editor_Increase_Resources_Tool

=============================
*/
int Editor_Change_Resource_Tool_Callback(FCoords& f, void* data, int curres) {
   Map* map=static_cast<Map*>(data);

   FCoords f1;
   Terrain_Descr* terr, *terd;
   int count=0;

   // This field
   terr=f.field->get_terr();
   terd=f.field->get_terd();
   bool is_valid_d = curres==terd->get_default_resources() || terd->is_resource_valid(curres) ? 1 : 0;
   bool is_valid_r = curres==terr->get_default_resources() || terr->is_resource_valid(curres) ? 1 : 0;
   if(terd->get_is()&TERRAIN_UNPASSABLE && is_valid_d) 
      count+=8; 
   else 
      count+= curres==terd->get_default_resources() || terd->is_resource_valid(curres) ? 1 : -1;
   if(terr->get_is()&TERRAIN_UNPASSABLE && is_valid_r) 
      count+=8;
   else 
      count+= curres==terr->get_default_resources() || terr->is_resource_valid(curres) ? 1 : -1;


   // If one of the neighbours is unpassable, count its resource stronger
   // top left neigbour
   map->get_neighbour(f, Map_Object::WALK_NW, &f1);
   terr=f1.field->get_terr();
   terd=f1.field->get_terd();
   is_valid_d = curres==terd->get_default_resources() || terd->is_resource_valid(curres) ? 1 : 0;
   is_valid_r = curres==terr->get_default_resources() || terr->is_resource_valid(curres) ? 1 : 0;
   if(terd->get_is()&TERRAIN_UNPASSABLE && is_valid_d) 
      count+=8; 
   else 
      count+= curres==terd->get_default_resources() || terd->is_resource_valid(curres) ? 1 : -1;
   if(terr->get_is()&TERRAIN_UNPASSABLE && is_valid_r) 
      count+=8;
   else 
      count+= curres==terr->get_default_resources() || terr->is_resource_valid(curres) ? 1 : -1;

   // top right neigbour
   map->get_neighbour(f, Map_Object::WALK_NE, &f1);
   terd=f1.field->get_terd();
   is_valid_r = curres==terr->get_default_resources() || terr->is_resource_valid(curres) ? 1 : 0;
   if(terd->get_is()&TERRAIN_UNPASSABLE && is_valid_r) 
      count+=8; 
   else 
      count+= curres==terd->get_default_resources() || terd->is_resource_valid(curres) ? 1 : -1;

   // left neighbour
   map->get_neighbour(f, Map_Object::WALK_W, &f1);
   terr=f1.field->get_terr();
   is_valid_r = curres==terr->get_default_resources() || terr->is_resource_valid(curres) ? 1 : 0;
   if(terr->get_is()&TERRAIN_UNPASSABLE && is_valid_r) 
      count+=8;
   else 
      count+= curres==terr->get_default_resources() || terr->is_resource_valid(curres) ? 1 : -1;

   if(count<=3) 
      return 0;
   else 
      return f.field->get_caps();
}

/*
===========
Editor_Increase_Resources_Tool::handle_click_impl()

decrease the resources of the current field by one if
there is not already another resource there.
===========
*/
int Editor_Increase_Resources_Tool::handle_click_impl(FCoords& fc, Map* map, Editor_Interactive* parent) {
   MapRegion mrc(map, fc, parent->get_fieldsel_radius());
   FCoords c;
   
   while(mrc.next(&c)) {
      Field* f=map->get_field(c);
      
      int res=f->get_resources();
      int amount=f->get_resources_amount();
      int max_amount=map->get_world()->get_resource(m_cur_res)->get_max_amount();

      amount+=m_changed_by;
      if(amount>max_amount) amount=max_amount;
     
      
      if((res==m_cur_res || !res) && Editor_Change_Resource_Tool_Callback(c,map,m_cur_res)) {
         // Ok, we're doing something. First remove the current overlays
         RGBColor clr;
         std::string str;
         if(res) {
            str=map->get_world()->get_resource(res)->get_editor_pic(f->get_resources_amount(), &clr);
            int picid=g_gr->get_picture(PicMod_Menu, str.c_str(), clr);
            map->get_overlay_manager()->remove_overlay(c,picid);

         }   
         if(!amount) { 
            f->set_resources(0,0);
         } else {
            f->set_resources(m_cur_res,amount);
            // set new overlay
            str=map->get_world()->get_resource(m_cur_res)->get_editor_pic(amount, &clr);
            int picid=g_gr->get_picture(PicMod_Menu, str.c_str(), clr);
            map->get_overlay_manager()->register_overlay(c,picid,4);
            map->recalc_for_field_area(c,0);
         }
      }
   }
   return parent->get_fieldsel_radius();
}
