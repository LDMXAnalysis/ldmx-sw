"""Module to write a basic detector GDML file

Attributes
----------
skeleton : str
    Multi-line string holding format for basic GDML file.
material_options : dict
    Dictionary of material names to their GDML definitions
    Materials must be named 'hunk_material' so that it can be
    referenced by the hunk volume
"""

import os

skeleton = """<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE gdml>
<gdml xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://service-spi.web.cern.ch/service-spi/app/releases/GDML/schema/gdml.xsd">
    <materials>
        <!-- We fill the non-hunk world with vaccuum -->
        <material name="vacuum" state="gas">
            <MEE unit="eV" value="19.2"/>
            <D unit="g/cm3" value="9.99999473841014e-09"/>
            <fraction n="1" ref="H"/>
        </material>
        <!-- We call whatever material we want to be in the hunk 'hunk_material' -->
        {hunk_material}
    </materials>

    <solids>
        <box name="world_box" x="{world_transverse}" y="{world_transverse}" z="{world_depth}" lunit="mm" />
        <box name="hunk_solid" x="{hunk_transverse}" y="{hunk_transverse}" z="{hunk_depth}" lunit="mm" />
    </solids>

    <structure>

      <!-- The EcalDarkBremFilter can be used if we name our volume in a specific way.
        That filter looks for the dark brem in all volumes that have 'volume' in the name
        AND one of 'W','Si','PCB', or 'CFMix'
        -->
        <volume name="hunk_W_volume">
            <materialref ref="hunk_material"/>
            <solidref ref="hunk_solid"/>
        </volume>

        <volume name="World">
            <materialref ref="vacuum"/>
            <solidref ref="world_box"/>
            
            <physvol name="hunk">
                <volumeref ref="hunk_W_volume"/>
                <position name="center" unit="mm" x="0" y="0" z="{hunk_depth}/2."/>
            </physvol> 

            <auxiliary auxtype="DetElem" auxvalue="Top"/>
        </volume>
    </structure>

    <userinfo>
        <auxiliary auxtype="DetectorVersion" auxvalue="999">
            <auxiliary auxtype="DetectorName" auxvalue="dark-brem-testing"/>
            <auxiliary auxtype="Description" 
                        auxvalue="A hunk of material to see how the dark brem process behaves."/>
        </auxiliary>
    </userinfo>  

    <setup name="Default" version="1.0">
        <world ref="World"/>
    </setup>
</gdml>
"""

material_options = {
    'tungsten' : """
        <isotope N="180" Z="74" name="W1800x270c100">
            <atom unit="g/mole" value="179.947"/>
        </isotope>
        <isotope N="182" Z="74" name="W1820x270c170">
            <atom unit="g/mole" value="181.948"/>
        </isotope>
        <isotope N="183" Z="74" name="W1830x270c210">
            <atom unit="g/mole" value="182.95"/>
        </isotope>
        <isotope N="184" Z="74" name="W1840x270c2b0">
            <atom unit="g/mole" value="183.951"/>
        </isotope>
        <isotope N="186" Z="74" name="W1860x270c2f0">
            <atom unit="g/mole" value="185.954"/>
        </isotope>
        <element name="W0x270bea0">
            <fraction n="0.0012" ref="W1800x270c100"/>
            <fraction n="0.265" ref="W1820x270c170"/>
            <fraction n="0.1431" ref="W1830x270c210"/>
            <fraction n="0.3064" ref="W1840x270c2b0"/>
            <fraction n="0.2843" ref="W1860x270c2f0"/>
        </element>
        <material name="hunk_material" state="solid">
            <T unit="K" value="293.15"/>
            <MEE unit="eV" value="727"/>
            <D unit="g/cm3" value="19.3"/>
            <fraction n="1" ref="W0x270bea0"/>
        </material>
        """
        }

def write(path, material, hunk_depth, hunk_transverse) :
    with open(path,'w') as gdml_f :
        gdml_f.write(skeleton.format(
            hunk_material = material_options[material],
            hunk_depth = hunk_depth,
            hunk_transverse = hunk_transverse,
            world_depth = 2*hunk_depth+10, #extra space for primary
            world_transverse = hunk_transverse+1
            ))
    return os.path.realpath(path)

if __name__ == '__main__' :
    import sys
    print(write('test_write.gdml',sys.argv[1], 500, 100))
