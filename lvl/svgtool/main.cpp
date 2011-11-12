/* The purpose of this program is to convert an svg file to a lvl file.
 * This will make level creation a little easier for me. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <string>
#include <map>
#include "tinyxml.h"


int main(int argc, char ** argv){
    if (argc != 2){
        printf("Usage: svg2lvl file.svg\n it will convert an svg with with cirlce and lines to a Zombie level file and print to the STDOUT.\n");
				printf("To save to a file just pipe the out put to a file.\n");
        return 1;
    }

    float h, w;
    float cx, cy, r;
    float x1, x2, y1, y2;

    TiXmlDocument doc(argv[1]);
    if (!doc.LoadFile()) return 1;

    TiXmlHandle hDoc(&doc);
    TiXmlElement* pElem;
    TiXmlHandle hRoot(0);

    {
        pElem = hDoc.FirstChildElement().Element();
        pElem->QueryFloatAttribute("width", &w);
        pElem->QueryFloatAttribute("height", &h);
        printf("hw %.1f %.1f\n", h, w);
        hRoot = TiXmlHandle(pElem);
    }
    pElem = hRoot.FirstChild("g").FirstChild().Element();
    pElem = pElem->NextSiblingElement();
    for(pElem; pElem; pElem = pElem->NextSiblingElement())
    {
        if(strcmp(pElem->Value(), "circle") == 0){
            const char *color=pElem->Attribute("fill");
            if(strcmp(color, "#e5e5e5") == 0){
                pElem->QueryFloatAttribute("cx", &cx);
                pElem->QueryFloatAttribute("cy", &cy);
                pElem->QueryFloatAttribute("r", &r);
                printf("sz %.1f %.1f %.1f\n", cx, h - cy, r);
            }
            else if(strcmp(color, "#FF0000") == 0){
                pElem->QueryFloatAttribute("cx", &cx);
                pElem->QueryFloatAttribute("cy", &cy);
                pElem->QueryFloatAttribute("r", &r);
                printf("h %.1f %.1f 0 0 %.1f %.1f\n", cx, h - cy, r, r*r*M_PI/25.2);
            }
            else if(strcmp(color, "#00bf5f") == 0){
                pElem->QueryFloatAttribute("cx", &cx);
                pElem->QueryFloatAttribute("cy", &cy);
                pElem->QueryFloatAttribute("r", &r);
                printf("z %.1f %.1f 0 0 %.1f %.1f\n", cx, h- cy, r, r*r*M_PI/25.2);
            }
            else if(strcmp(color, "#ffff00") == 0){
                pElem->QueryFloatAttribute("cx", &cx);
                pElem->QueryFloatAttribute("cy", &cy);
                pElem->QueryFloatAttribute("r", &r);
                printf("p %.1f %.1f 0 0 %.1f %.1f\n", cx, h - cy, r, r*r*M_PI/25.2);
            }

        }
        else if(strcmp(pElem->Value(), "line") == 0){
            pElem->QueryFloatAttribute("x1", &x1);
            pElem->QueryFloatAttribute("x2", &x2);
            pElem->QueryFloatAttribute("y1", &y1);
            pElem->QueryFloatAttribute("y2", &y2);
            printf("w %.1f %.1f %.1f %.1f\n", x1, h-y1, x2, h - y2); 
        }
    }    

    
    return 0;
}
