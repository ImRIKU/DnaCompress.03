for file in "$@"; do
    echo -e "Processing file: $file\n\n\n\n\nGECO OPERATION"
    ./geco/src/GeCo -l 5 ../dna/"$file"
    
    echo -e "\nDECOMPRESSION"
    ./geco/src/GeDe ../dna/"$file".co

    echo -e "\n\n\nGECO2 OPERATION"
    ./geco2/src/GeCo2 -v -l 5 ../dna/"$file"
    
    echo -e "\nDECOMPRESSION"
    ./geco2/src/GeDe2 ../dna/"$file".co
    
    echo -e "\n\n\nGECO3 OPERATION"
    ./geco3/src/GeCo3 -l 1 -lr 0.06 -hs 8 ../dna/"$file"
    
    echo -e "\nDECOMPRESSION"
    ./geco3/src/GeDe3 ../dna/"$file".co
    
    echo -e "\n\n\nJARVIS OPERATION"
    ./jarvis/src/JARVIS -v -l 4 ../dna/"$file"
    
    echo -e "\nDECOMPRESSION"
    ./jarvis/src/JARVIS -v -d ../dna/"$file".jc
done
