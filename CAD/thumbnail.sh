# https://github.com/unlimitedbacon/stl-thumb
# sudo apt install ./stl-thumb_0.5.0_amd64.deb

# NOTE: be in the same directory wqhen you are running this.
echo "# CAD Appendix" > README.md

echo -e "\n<!---  AUTOMATICALLY GENERATED FILE. DONT EDIT THIS FILE DIRECTLY. EDIT thumbnail.sh INSTEAD --> " >> README.md

echo "STL files can be opened using any 3D printer slicer software, at Infotiv we use 'Ideamaker'. [Download here](https://www.raise3d.com/ideamaker/). Use a slicer configuration for an 'Raise3d E2' 3d printer. PLA material choice is fine if you don't leave autonomous platform in a hot car during the day. A base slicer profile for the Raise3D E2 can be found in the CAD profile. It is a nice starting point." >> README.md

for i in $(find .  -type f -name "*.stl"); do
    echo "$i"
    # stl-thumb "$i" "$i-thumbnail.png"
    echo -e "\n$i:\n\n![$i]($i-thumbnail.png){ width=70% }"  >> README.md
done
