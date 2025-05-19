# Notice d’installation et de compilation du projet sur macOS

## Étapes d’installation

### 1. Installer `Homebrew` (si ce n’est pas déjà fait) :

```bash
/bin/bash -c “$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)”
```

###  2. Installer le compilateur ARM :

```bash
brew tap ArmMbed/homebrew-formulae
brew install arm-none-eabi-gcc
```

### 3. Vérifier que le compilateur est installé :

```bash
which arm-none-eabi-gcc
```

> Le chemin affiché devrait être quelque chose comme : _/opt/homebrew/bin/arm-none-eabi-gcc_
	
### 4. Configurer Eclipse pour qu’il trouve `arm-none-eabi-gcc` :

- Aller dans : Eclipse > Preferences > C/C++ > Build > Environment
- Cliquer sur “Add…”
- Name : PATH
- Value : /opt/homebrew/bin:${PATH}
- Cocher “Append variables to native environment”

### 5. Importer le projet dans Eclipse :

- File > Import > General > Existing Projects into Workspace
- Sélectionner le dossier du projet
- Valider

### 6. Compiler le projet :

- Menu Project > Clean
- Puis Project > Build Project

___


_Exemple de trace dans la Build Console :_

```
13:29:22 **** Rebuild of configuration Debug for project mi11_ntr_full_p25_emu ****
Info: Internal Builder is used for build
arm-none-eabi-gcc -O0 -g -Wall -c -fmessage-length=0 -ffreestanding -mcpu=cortex-m7 -mfloat-abi=softfp -o kernel/delay.o ../kernel/delay.c 
arm-none-eabi-gcc -O0 -g -Wall -c -fmessage-length=0 -ffreestanding -mcpu=cortex-m7 -mfloat-abi=softfp -o hwsupport/stm_gpio.o ../hwsupport/stm_gpio.c 
arm-none-eabi-gcc -O0 -g -Wall -c -fmessage-length=0 -ffreestanding -mcpu=cortex-m7 -mfloat-abi=softfp -o kernel/chonogram.o ../kernel/chonogram.c 
arm-none-eabi-gcc -O0 -g -Wall -c -fmessage-length=0 -ffreestanding -mcpu=cortex-m7 -mfloat-abi=softfp -o kernel/fifo.o ../kernel/fifo.c 
arm-none-eabi-gcc -O0 -g -Wall -c -fmessage-length=0 -ffreestanding -mcpu=cortex-m7 -mfloat-abi=softfp -o hwsupport/cmsdk_apb_uart.o ../hwsupport/cmsdk_apb_uart.c 
arm-none-eabi-gcc -c -g -o hwsupport/vectors.o ../hwsupport/vectors.S 
arm-none-eabi-gcc -O0 -g -Wall -c -fmessage-length=0 -ffreestanding -mcpu=cortex-m7 -mfloat-abi=softfp -o kernel/noyau_prio.o ../kernel/noyau_prio.c 
arm-none-eabi-gcc -O0 -g -Wall -c -fmessage-length=0 -ffreestanding -mcpu=cortex-m7 -mfloat-abi=softfp -o kernel/init.o ../kernel/init.c 
arm-none-eabi-gcc -O0 -g -Wall -c -fmessage-length=0 -ffreestanding -mcpu=cortex-m7 -mfloat-abi=softfp -o io/serialio.o ../io/serialio.c 
arm-none-eabi-gcc -O0 -g -Wall -c -fmessage-length=0 -ffreestanding -mcpu=cortex-m7 -mfloat-abi=softfp -o kernel/sem.o ../kernel/sem.c 
arm-none-eabi-gcc -O0 -g -Wall -c -fmessage-length=0 -ffreestanding -mcpu=cortex-m7 -mfloat-abi=softfp -o kernel/noyau_file_prio.o ../kernel/noyau_file_prio.c 
arm-none-eabi-gcc -O0 -g -Wall -c -fmessage-length=0 -ffreestanding -mcpu=cortex-m7 -mfloat-abi=softfp -o hwsupport/cortex.o ../hwsupport/cortex.c 
arm-none-eabi-gcc -O0 -g -Wall -c -fmessage-length=0 -ffreestanding -mcpu=cortex-m7 -mfloat-abi=softfp -o noyau_test_prio.o ../noyau_test_prio.c 
arm-none-eabi-gcc -O0 -g -Wall -c -fmessage-length=0 -ffreestanding -mcpu=cortex-m7 -mfloat-abi=softfp -o io/TERMINAL.o ../io/TERMINAL.c 
arm-none-eabi-gcc -nostdlib -T ../kernel/ld.x -mcpu=cortex-m7 -mfloat-abi=softfp -o mi11_ntr_full_p25_emu hwsupport/cmsdk_apb_uart.o hwsupport/cortex.o hwsupport/stm_gpio.o hwsupport/vectors.o io/TERMINAL.o io/serialio.o kernel/chonogram.o kernel/delay.o kernel/fifo.o kernel/init.o kernel/noyau_file_prio.o kernel/noyau_prio.o kernel/sem.o noyau_test_prio.o -lgcc 

13:29:24 Build Finished. 0 errors, 0 warnings. (took 2s.86ms)
```
