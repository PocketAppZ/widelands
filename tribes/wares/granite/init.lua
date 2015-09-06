dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "granite",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Granite"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "granite"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 20,
		barbarians = 20,
		empire = 30
	},
   preciousness = {
		atlanteans = 5,
		barbarians = 5,
		empire = 0
	},

   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 8 },
      },
   }
}
