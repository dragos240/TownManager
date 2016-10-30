#include <3ds.h>
#include <sf2d.h>
#include <sftd.h>
#include <sfil.h>

sf2d_texture* topbar;
sf2d_texture* tm;

void ui_init(){
	sf2d_set_clear_color(RGBA8(0x7e, 0x76, 0x29, 0xFF));

	topbar = sfil_load_PNG_file("romfs:/topbar.png", SF2D_PLACE_RAM);
	tm = sfil_load_PNG_file("romfs:/tm.png", SF2D_PLACE_RAM);
}

void ui_fini(){
	sf2d_free_texture(topbar);
	sf2d_free_texture(tm);
}

void ui_frame(){
	sf2d_draw_texture(topbar, 0, 0);
	sf2d_draw_texture(tm, 289, 18);
}
