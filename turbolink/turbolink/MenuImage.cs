using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Drawing;

namespace turbolink
{
    class MenuImage
    {

        const int plan_w = 64;//512/8
        const int screen_w = 40;//320/8
        const int screen_h = 28;//224/8

        public static void makeImage(string path, byte []vram, byte[] pal8)
        {


            Bitmap pic = new Bitmap(screen_w * 8, screen_h * 8);

            UInt16[] pal16 = getPal16(pal8);
            int[] pal32 = getPal32(pal16);
            UInt16[] tilemap = getTilemap(vram);


            for (int i = 0; i < 320 * 224; i++)
            {
                int x = i % 320;
                int y = i / 320;
                int tile_ptr = x / 8 + y / 8 * screen_w;
                int tile_pal = tilemap[tile_ptr] >> 12;
                int tile_idx = tilemap[tile_ptr] & 0xfff;
                int tile_pixel = getPixel(vram, tile_idx, x, y);

                int rgb = pal32[tile_pal * 16 + tile_pixel];

                pic.SetPixel(x, y, Color.FromArgb(rgb));
            }

            pic.Save(path);
        }

        static int getPixel(byte[] vram, int tile_idx, int x, int y)
        {
            int pixel = 0;
            x %= 8;
            y %= 8;

            int ptr = tile_idx * 32;
            ptr += y * 2;

            int bit_ptr = 7 - x;
            int[] bits = new int[4];

            bits[0] = (vram[ptr + 0] >> bit_ptr) & 1;
            bits[1] = (vram[ptr + 1] >> bit_ptr) & 1;
            bits[2] = (vram[ptr + 16] >> bit_ptr) & 1;
            bits[3] = (vram[ptr + 17] >> bit_ptr) & 1;

            pixel = (bits[3] << 3) | (bits[2] << 2) | (bits[1] << 1) | (bits[0] << 0);

            return pixel;
        }

        static UInt16[] getTilemap(byte []vram)
        {
            
            UInt16[] map = new UInt16[screen_w * screen_h];

            for (int y = 0; y < screen_h; y++)
            {
                for (int x = 0; x < screen_w; x++)
                {
                    map[x + y * screen_w] = (UInt16)(vram[(x + y * plan_w) * 2 + 0] | (vram[(x + y * plan_w) * 2 + 1] << 8));
                }
            }

            return map;
        }

        static UInt16[] getPal16(byte[] pal8)
        {
            UInt16[] pal16 = new UInt16[pal8.Length / 2];

            for(int i = 0;i < pal16.Length; i++)
            {
                pal16[i] = (UInt16)(pal8[i * 2 + 0] | (pal8[i * 2 + 1] << 8));
            }

            return pal16;
        }

        static int[] getPal32(UInt16 []pal16)
        {
            int[] pal32 = new int[pal16.Length];

            for(int i = 0;i < pal32.Length; i++)
            {
                int r = (pal16[i] >> 3) & 7;
                int g = (pal16[i] >> 6) & 7;
                int b = (pal16[i] >> 0) & 7;

                r <<= 5;
                g <<= 5;
                b <<= 5;

                int alpha = 0xff0000;
                alpha <<= 8;


                pal32[i] = alpha | (r << 16) | (g << 8) | (b << 0);
                
            }

            

            return pal32;
        }

    }
}
