g++ -O3 modules/models/bot_level_1.cpp -o modules/models/bot_level_1
g++ -O3 modules/models/bot_level_2.cpp -o modules/models/bot_level_2
g++ -O3 modules/models/bot_level_3.cpp -o modules/models/bot_level_3

# Biên dịch bot final (kế thừa bot 3)
g++ -O3 modules/models/bot_final.cpp -o modules/models/bot_final

chmod +x modules/models/bot_level_1
chmod +x modules/models/bot_level_2
chmod +x modules/models/bot_level_3
chmod +x modules/models/bot_final
