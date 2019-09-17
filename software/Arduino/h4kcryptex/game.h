/***************************************************************************//**

  @file         was main.c now lsh.ino

  @author       Stephen Brennan

  @date         Thursday,  8 January 2015

  @brief        LSH (Libstephen SHell)
  Link:         https://brennan.io/2015/01/16/write-a-shell-in-c/

  Edited for Arduino by Mads Aasvik February 2, 2018
  Link:         https://www.norwegiancreations.com/2018/02/creating-a-command-line-interface-in-arduinos-serial-monitor/

  Adapted and modified by Hak4Kidz for Cryptex badge June 23, 2019

*******************************************************************************/

// declare included libraries
#include <Wire.h>
#include <pgmspace.h>
#include <SPI.h>
#include <TFT_eSPI.h>
//#include "SPIFFS.h"

#define LINE_BUF_SIZE 128   //Maximum input string length
#define ARG_BUF_SIZE 64     //Maximum argument string length
#define MAX_NUM_ARGS 8      //Maximum number of arguments
#define Addr_VCC 0x78       //7 bit format is 0x3F;;

// Global variables
void (*state)() = NULL;
bool solved0 = false; // Door is locked
bool solved1 = false; // wall 1 chall
bool solved2 = false; // wall 2 chall
bool solved3 = false; // wall 3 chall
bool solved4 = false; // wall 4 chall
bool solved5 = false; // wall 5 chall
bool error_flag = false;
bool hasScroll = false;

TFT_eSPI tft = TFT_eSPI();

float badge_ver = 0.90;
int hackcnt = 0;

char line[LINE_BUF_SIZE];
char args[MAX_NUM_ARGS][ARG_BUF_SIZE];
String location = "center";

//Wire.begin(16, 17);               // tell MCU which pins to assdress the LED driver with
//Wire.setClock(400000);            // set I2C to 400kHz

//Function declarations
int cmd_help();
int cmd_blink();
int cmd_look();
int cmd_wall();
int cmd_hack();
int cmd_cntr();
int cmd_intro();
int cmd_loc();
int cmd_exit();

// used to fade lights on and off
byte PWM_Gamma64[64] =
{
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0b, 0x0d, 0x0f, 0x11, 0x13, 0x16,
  0x1a, 0x1c, 0x1d, 0x1f, 0x22, 0x25, 0x28, 0x2e,
  0x34, 0x38, 0x3c, 0x40, 0x44, 0x48, 0x4b, 0x4f,
  0x55, 0x5a, 0x5f, 0x64, 0x69, 0x6d, 0x72, 0x77,
  0x7d, 0x80, 0x88, 0x8d, 0x94, 0x9a, 0xa0, 0xa7,
  0xac, 0xb0, 0xb9, 0xbf, 0xc6, 0xcb, 0xcf, 0xd6,
  0xe1, 0xe9, 0xed, 0xf1, 0xf6, 0xfa, 0xfe, 0xff
};

//List of functions pointers corresponding to each command
int (*commands_func[])() {
  &cmd_help,
  &cmd_blink,
  &cmd_look,
  &cmd_wall,
  &cmd_cntr,
  &cmd_hack,
  &cmd_intro,
  &cmd_loc,
  &cmd_exit
};

//List of command names
char *commands_str[] = {
  "help",
  "blink",
  "look",
  "wall",
  "center",
  "hack",
  "intro",
  "location"
};

//List of LOOk sub command options
char *look_cmd_args[] = {
  "door",
  "computer",
  "picture",
  "workstation",
  "wall",
  "cylinder",
  "note",
  "scroll"
};

//List of LED sub command names
char *led_args[] = {
  "left",
  "right"
};

//List of WALL sub command names
char *wall_cmd_args[] = {
  "kick",
  "1",
  "2",
  "3",
  "4",
  "5"
};

// The command to return to center
char *cntr_args[] = {
  "center"
};

//List of HACK sub command names
char *hack_args[] = {
  "keypad",
  "code",
  "database",
  "picture",
  "computer",
  "cryptex"
};

//List of database hashes
char *db_hashes[] = {
  "5f4dcc3b5aa765d61d8327deb882cf99",
  "a8f6830bce790a8a67fc2e84e12093ba",
  "9095220ABA77AA81AAD3B435B51404EE",
  "00C8ABD57950A7FEAAD3B435B51404EE",
  "E52CAC67419A9A224A3B108F3FA6CB6D",
  "7B96B77A223162B1AAD3B435B51404EE",
  "74C16041164A1F77AAD3B435B51404EE",
  "EAE53D3A9E5C403DAD6D0B05DE86D7F2",
  "24CC4D247BF23904B2F42606040369A4",
  "DAD9762D4704E187E019800100D84CA3",
  "6D853CFC6A187D81415DDDCE1A678850",
  "82FFAAA51588F4A2778CF94BB2F640AA",
  "5EBE7DFA074DA8EE8AEF1FAA2BBDE876",
  "408E50575E23A3162BCF409C196D0329",
  "469CE7E90C0E4FF2F162F6E4094C3D8B",
  "D03BEE44B5333672A7D74248A1C9366A",
  "1BFD6569011B0301E7992F653E325888",
  "dfb3dd95fb552c894d286771061ae399",
  "d8578edf8458ce06fbc5bb76a58c5ca4",
  "0571749e2ac330a7455809c6b0e7af90",
  "bdce6293098a6a3cef4c9a648a22d0ff",
  "25f9e794323b453885f5181f1b624d0b",
  "8afa847f50a716e64932d995c8e7435a",
  "40be4e59b9a2a2b5dffb918c0e86b3d7",
  "f116acfe9147494063e58da666d1d57e"
};

void cryptex()
{
  Serial.println("");
  Serial.println("                 ,*&*%,                                             ");
  Serial.println("               */%(/#(##*,*//*                                        ");
  Serial.println("             *%./#(#/,###, ..,  /                                 ");
  Serial.println("           ,&#(#/.%/ /.   /*    .                                 ");
  Serial.println("          ,&%%/,/,(#,%   **# .  ** .  *                           ");
  Serial.println("         .&/&(/#.#*  .  *, (  *# *   (.  ,*                          ");
  Serial.println("         ***%%/*#*  * *. ,  ,    ./       ,  ,,    .*,%&%*               ");
  Serial.println("         **#(#(*&. % ..   (.  ( ./ *   .. .. ( */&%(#/##(//*/,        ");
  Serial.println("         /*(/#*%% ,  ##   ,  #  *   % .   #  *,&%##((*/#(**/(##(*  ");
  Serial.println("         /(/((/&( . *, ( .. #  ,#     (    ,,%((##/*,/*,*##/(%(%&/   ");
  Serial.println("          /(%(/ # (  *./ #  *  *  .     ,.&%,(*/.*(,/%#(#(((#%**&/    ");
  Serial.println("           #*%(*  *#   * /  (  % ,  ,, ,.%....#./%,*(((#(((#%#%##(       ");
  Serial.println("              #%/& / # * * # *( #  , * *,@%,(/.##./%@#/(%%*/%/(#(%#   ");
  Serial.println("              .,(#&/,,*,/   /. ( *.   /*/%#.///%./%%##%*&//*##(%%&/    ");
  Serial.println("           ..,,***//(#*./( ( / # * %  ,/@/(#%,*#&((##&/%*&/%(&%@#/   ");
  Serial.println("            ...,,,,,,***//(,/.,  .. % */@#%/(**//&&((%(%(&/##(##%( ");
  Serial.println("               .......,,,,***///, / ,.,/@%%///#/(#((##%///%/###%%  ");
  Serial.println("                    ......,,,,***//*.* /@&/(*##/%%##%*#&/(###/#%.      ");
  Serial.println("                         .....,,,,**//(*/@%(##%/##(#(#(####/(%(    ");
  Serial.println("                           ......,,,,***/(@&%*%//#%/######&(...    ");
  Serial.println("                               ......,,,,***/((%(/##%(&&(%&(,.     ");
  Serial.println("                                   ....,,,,***//((//%&&/(/,..      ");
  Serial.println("                                       ...,,,,,******,,...         ");
}

void note()
{
  //              0123456789012345678901234567890123456789012345678901234567890123456789
  Serial.println("\n     Tib ysrf dtppubz ms p ocslqhkvm dqkref ca whb pwwhqs Gen Csrzn gqt kit");
  Serial.println("     2003 oryee Ujd Dp Wkrcj Drhe, fbqstjoi e pqsvebeb xeueu wvef ur kifb ");
  Serial.println("     udcsbv qettljet. Jv ms p xrud gqtqef gtsm Hscdk msabtót, \“ikhdbo, ");
  Serial.println("     udcsbv\” enf Elwio drhey; \“pq epu ukwlb gru tiju hewjfd\” sjofd iu vuds ");
  Serial.println("     \“uic vcjbqge qg fuylurcohz vs psqvdcu jqiosnlwiqo yuiuucr oo ujd ");
  Serial.println("     cqoveiobg vcsqbc os drhey”. Ujd fjsuw pizumcpe fuyluca wpt fuepuch bz ");
  Serial.println("     Kwvtjo Nmrm Ocyiot kr 2004. Utb vke dsabtby dedhb iemb drqmpog ‘kadm ");
  Serial.println("     fuyluca allbd’ tq vqcodm kw’s tbfueu.\n");
}

void scroll()
{
  Serial.println("     \n\nFinal sekrit of the cryptex lies before you,");
  Serial.println("     Seek beyond the lights of blue.");
  Serial.println("     Final reward lies amongst the binary,");
  Serial.println("     On the bit walk, a single source of data ternary.\n\n");
}
byte rndIndex;

int num_commands = sizeof(commands_str) / sizeof(char *);

char buffer[80]; //screen character width

void read_line() {
  String line_string;

  while (!Serial.available());

  if (Serial.available()) {
    line_string = Serial.readStringUntil('\n');
    if (line_string.length() < LINE_BUF_SIZE) {
      line_string.toCharArray(line, LINE_BUF_SIZE);
      Serial.println(line_string);
    }
    else {
      Serial.println("Input string too long.");
      error_flag = true;
    }
  }
}

void parse_line() {
  char *argument;
  int counter = 0;

  argument = strtok(line, " ");

  while ((argument != NULL)) {
    if (counter < MAX_NUM_ARGS) {
      if (strlen(argument) < ARG_BUF_SIZE) {
        strcpy(args[counter], argument);
        argument = strtok(NULL, " ");
        counter++;
      }
      else {
        Serial.println("Input string too long.");
        error_flag = true;
        break;
      }
    }
    else {
      break;
    }
  }
}

int execute() {
  for (int i = 0; i < num_commands; i++) {
    if (strcmp(args[0], commands_str[i]) == 0) {
      return (*commands_func[i])();
    }
  }

  Serial.println("Invalid command. Type \"help\" without pressing ENTER.");
  return 0;
}


void help_help() {
  Serial.println("\nYou appear to need all the help you can get. Perhaps begin by making a friend?");
  Serial.println("The following commands are available:");

  for (int i = 0; i < num_commands; i++) {
    Serial.print("  ");
    Serial.println(commands_str[i]);
  }
  Serial.println("");
  Serial.println("You can for instance type \"help look\" for more info on the LOOK command.\n");
}

void help_blink() {
  Serial.println("Control when Tinker blinks ");
  Serial.println("  blink left");
  Serial.println("  blink right\n");
}

void help_look() {
  Serial.println("\nUse this command to receive a description of what's in your immediate view or an object.\n");
  Serial.println("Speak like Tarzan. For example: \"look wall\" to, you know...look at a wall.");
}

void help_wall() {
  Serial.println("\nUse this command to advance to a wall from the \'center\' of the room.");
  Serial.println("Example usage\: wall 0\n");
}

void help_cntr() {
  Serial.println("\nUse this command to center yourself and progress in the escape room.\n");
}

int help_hack() {
  Serial.println("\nHacking is hard.");
  Serial.println("Be creative and try to hack the planet.\n");
}

int help_location() {
  Serial.println("\nUse this command to figure out where you are when you are lost.");
  Serial.println("Example usage\: location\n");
}

int cmd_help() {
  if (args[1] == NULL) {
    help_help();
  }
  else if (strcmp(args[1], commands_str[0]) == 0) {
    help_help();
  }
  else if (strcmp(args[1], commands_str[1]) == 0) {
    help_blink();
  }
  else if (strcmp(args[1], commands_str[2]) == 0) {
    help_look();
  }
  else if (strcmp(args[1], commands_str[3]) == 0) {
    help_wall();
  }
  else if (strcmp(args[1], commands_str[4]) == 0) {
    help_cntr();
  }
  else if (strcmp(args[1], commands_str[5]) == 0) {
    help_hack();
  }
  else if (strcmp(args[1], commands_str[6]) == 0) {
    help_location();
  }
  else {
    help_help();
  }
}

// function to address LED driver to turn on LEDs
void IS_IIC_WriteByte(uint8_t Dev_Add, uint8_t Reg_Add, uint8_t Reg_Dat)
{
  Wire.begin(16, 17);
  Wire.beginTransmission(Dev_Add / 2);   // start transmitting
  Wire.write(Reg_Add);                   // sends regaddress
  Wire.write(Reg_Dat);                   // sends regaddress
  Wire.endTransmission(Dev_Add / 2);                // stop transmitting
}

void LBlueEye(void)
{
  IS_IIC_WriteByte(Addr_VCC, 0x03, 0xff);     //set  D1 blue
}

void LBlueEyeClose(void)
{
  int8_t j = 0;
  for (j = 63; j >= 0; j--)
  {

    IS_IIC_WriteByte(Addr_VCC, 0x03, PWM_Gamma64[j]); // fade out
    IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);         //update  PWM  &  congtrol  registers
    delay(5);
  }
}

void RBlueEye(void)
{
  IS_IIC_WriteByte(Addr_VCC, 0x06, 0xff);     //set  D2 blue
}

void RBlueEyeClose(void)
{
  int8_t j = 0;
  for (j = 63; j >= 0; j--)
  {
    IS_IIC_WriteByte(Addr_VCC, 0x06, PWM_Gamma64[j]); // fade out
    IS_IIC_WriteByte(Addr_VCC, 0x25, 0x00);         //update  PWM  &  congtrol  registers
    delay(5);
  }
}

int cmd_blink() {
  // TO DO: figure out why it crashes on second blink
  if (strcmp(args[1], led_args[0]) == 0) {
    Serial.println("Blinking the left eye.");
    LBlueEyeClose();
    delay(100);
    LBlueEye();
  }
  else if (strcmp(args[1], led_args[1]) == 0) {
    Serial.println("Blinking the right eye.");
    RBlueEyeClose();
    delay(100);
    RBlueEye();
  }
  else {
    Serial.println("You have to ask nicely to make Tinker blink.");
    Serial.println("\nInvalid command. Type \"help blink\" to see how to use the BLINK command.");
  }
}

int cmd_look() {
  if (strcmp(args[1], look_cmd_args[0]) == 0)  {
    // add something to test for 'wall wall' and return 0
    if (location == "door") {
      Serial.print("\nUsing your orbits, ");
      Serial.println("the door looks like it's made of a sturdy metal and is very smooth.");
      Serial.println("Impossible to bypass other than using the keypad on the right.\n");
    }
    else {
      Serial.println("\nYou are not standing in front of a door.\n");
    }
  }
  else if (strcmp(args[1], look_cmd_args[1]) == 0) {
    if (location == "wall5") {
      //              01234567890123456789012345678901234567890123456789012345678901234567890123456789
      Serial.println("\nLooking at the computer you see the following message on the display.\n");
      Serial.println("             ____________________________________________________");
      Serial.println("            /                                                    \\ ");
      Serial.println("           |    _____________________________________________     |");
      Serial.println("           |   | Enter password to unlock.                   |    |");
      Serial.println("           |   |  $>_                                        |    |");
      Serial.println("           |   |                                             |    |");
      Serial.println("           |   |                                             |    |");
      Serial.println("           |   |                                             |    |");
      Serial.println("           |   |                                             |    |");
      Serial.println("           |   |                                             |    |");
      Serial.println("           |   |                                             |    |");
      Serial.println("           |   |                                             |    |");
      Serial.println("           |   |_____________________________________________|    |");
      Serial.println("           |                                                      |");
      Serial.println("            \\_____________________________________________________/");
      Serial.println("                   \\_______________________________________/");
      Serial.println("                _______________________________________________");
      Serial.println("             _-'    .-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.  --- `-_");
      Serial.println("          _-'.-.-. .---.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.--.  .-.-.`-_");
      Serial.println("       _-'.-.-.-. .---.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-`__`. .-.-.-.`-_");
      Serial.println("    _-'.-.-.-.-. .-----.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-----. .-.-.-.-.`-_");
      Serial.println(" _-'.-.-.-.-.-. .---.-. .-----------------------------. .-.---. .---.-.-.-.`-_");
      Serial.println(":-----------------------------------------------------------------------------:");
      Serial.println("`---._.-----------------------------------------------------------------._.---'");
      Serial.println("\n");
    }
    else {
      Serial.println("\nYou are not standing in front of a computer.\n");
    }
  }
  else if (strcmp(args[1], look_cmd_args[2]) == 0) {
    if (location == "wall2") {
      //              01234567890123456789012345678901234567890123456789012345678901234567890123456789
      Serial.println("\nLooking more closely at the picture, it has an eerie familiarity.");
      Serial.println("You know you've seen this before but where?\n");
    }
    else {
      Serial.println("\nYou are not standing in front of a picture.\n");
    }
  }
  else if (strcmp(args[1], look_cmd_args[3]) == 0) {
    if (location == "wall5") {
      //              01234567890123456789012345678901234567890123456789012345678901234567890123456789
      Serial.println("\nAs you look at the workstation, you notice something about it...");
      delay(1000);
      Serial.println("\nabsolutely nothing.");
      Serial.println("\nHave you never seen a workstation before?\n");
    }
    else {
      Serial.println("\nYou are not standing in front of a workstation.\n");
    }
  }
  else if (strcmp(args[1], look_cmd_args[4]) == 0) {
    if (atoi(args[2]) == 0) {
      if (location == "door") {
        //              01234567890123456789012345678901234567890123456789012345678901234567890123456789
        Serial.println("\nUsing your orbits, you see a locked door in the wall. To the right is a numeric");
        Serial.println("keypad, which you deduced will unlock the door. The keypad has numbers from 0-9,");
        Serial.println("much like an old timey push button phone.\n");
      }
      else {
        Serial.print("\nYou are not standing in front of wall ");
        Serial.print(args[2]);
        Serial.println(".\n");
      }
    }
    else if (atoi(args[2]) == 1) {
      if (location == "wall1") {
        //              01234567890123456789012345678901234567890123456789012345678901234567890123456789
        Serial.println("\nWhy are you looking at this wall?\n");
        Serial.println("Go solder something and show it to Heal. I bet he'll like it.\n");
      }
      else {
        Serial.print("\nYou are not standing in front of wall ");
        Serial.print(args[2]);
        Serial.println(".\n");
      }
    }
    else if (atoi(args[2]) == 2) {
      if (location == "wall2") {
        //             01234567890123456789012345678901234567890123456789012345678901234567890123456789
        Serial.println("\nAs you look more closely at wall 2, you realize the art get blurrier.");
        Serial.println("So you move back and it gets clearer. Perhaps try using the link?");
      }
      else {
        Serial.print("\nYou are not standing in front of wall ");
        Serial.print(args[2]);
        Serial.println(".\n");
      }
    }
    else if (atoi(args[2]) == 3) {
      if (location == "wall3") {
        //              01234567890123456789012345678901234567890123456789012345678901234567890123456789
        Serial.print("\nLooking more closely at the flowing masked data, you begin to make out a structure");
        Serial.println("of the data...\n");
        Serial.println("HINT: You will need to hack this base of data.");
      }
      else {
        Serial.print("\nYou are not standing in front of wall ");
        Serial.print(args[2]);
        Serial.println(".\n");
      }
    }
    else if (atoi(args[2]) == 4) {
      if (location == "wall4") {
        //              01234567890123456789012345678901234567890123456789012345678901234567890123456789
        Serial.println("\nUsing your orbits, wall 4 has a fashionable console table and decorative");
        Serial.println("cylinder. Did you look at it yet?\n");
      }
      else {
        Serial.print("\nYou are not standing in front of wall ");
        Serial.print(args[2]);
        Serial.println(".\n");
      }
    }
    else if (atoi(args[2]) == 5) {
      if (location == "wall5") {
        //              01234567890123456789012345678901234567890123456789012345678901234567890123456789
        Serial.println("\nLooking more closely at this wall, there is an old timey computer. The computer");
        Serial.println("is powered on with a message displayed. Try looking at the computer to read the");
        Serial.println("message.   The workstation is very sturdy and made of dark walnut from the");
        Serial.println("Ohio Valley. There is a humming computer resting on the workstation. Oddly,");
        Serial.println("the workstation doesn't have a chair. You'll have to stand to use it.\n");
      }
      else {
        Serial.print("\nYou are not standing in front of wall ");
        Serial.print(args[2]);
        Serial.println(".\n");
      }
    }
    else {
      Serial.print("\nYou've had too much drink, there's no such wall ");
      Serial.print(args[2]);
      Serial.println(". SQUIRREL!!\n");
    }
  }
  else if (strcmp(args[1], look_cmd_args[5]) == 0) {
    if (location == "wall4") {
      cryptex();
      Serial.println("\nOn closer inspection, the cylinder is a genuine cryptex! You");
      Serial.println("found it! Now you wonder...what's the key to unlock it's secret?");
      Serial.println("\nLifting the cryptex to inspect it, you find a note underneath.\n");
    }
    else {
      Serial.println("\nNo cryptex here. Try over at wall 4.\n");
    }
  }
  else if (strcmp(args[1], look_cmd_args[6]) == 0) {
    if (location == "wall4") {
      note();
    }
    else {
      Serial.println("\nNo note here. Try over at wall 4.\n");
    }
  }
  else if (strcmp(args[1], look_cmd_args[7]) == 0) {
    if (hasScroll) {
      scroll();
    }
    else {
      Serial.println("\nNo scroll here. Heard rumors of one, though.\n");
    }
  }
  else {
    Serial.print("\nThere is no ");
    Serial.print(args[1]);
    Serial.println(" to look at.");
    Serial.println("\nInvalid command. Type \"help look\" to see how to use the LOOK command.\n");
  }
}

int cmd_exit() {
  Serial.println("\nCongratulations! You have escaped from the Hak4Kidz Cryptex.");
  Serial.println("bit.ly\/2Z5LZ7r \n");

  while (1);
}

void tftBlank() {
  tft.begin();                       // enable the LCD using tft
  tft.fillScreen(ILI9341_BLACK);
}

int cmd_hack() {
  if (location == "door") {
    if (strcmp(args[1], hack_args[0]) == 0) {
      if (atoi(args[2]) > 0) {
        Serial.print("\nHacking the door keypad, you enter code ");
        Serial.print(args[2]);
        Serial.println(" with anticipation.\n");
        delay(1500);
        if (atoi(args[2]) == 312312) {
          Serial.println("A solenoid lock clicks, the door opens!!!");
          delay(1000);
          Serial.println("Time to celebrate, you beat the Hak4Kidz Cryptex virtual escape room!!!\n");
          solved0 = true;
        }
        else {
          Serial.println("But end up feeling disappointment.\n");
        }
      }
    }
    else if (strcmp(args[1], hack_args[0]) != 0) {
      Serial.print("\nAttemping to hack ");
      Serial.print(args[1]);
      Serial.println(" fails. Try hacking a different way.\n");
    }
  }
  else if (location == "wall1") {
    if (strcmp(args[1], hack_args[1]) == 0) {
      Serial.println("\nHack hardware.\n");
      if (atoi(args[2]) > 0) {
        Serial.print("You enter the numeric sequence...");
        delay(1500);
        if (atoi(args[2]) == 1123581321) {
          Serial.println("and it's correct!\n");
          Serial.println("Nice job entering the beginning of the Fibonacci sequence.\n");
          solved1 = true;
        }
        else {
          Serial.println("but it's incorrect.\nThis isn't rocket science. Hack harder.\n");
          //Serial.println("Yeah, that's right! I said it!!\n");
        }
      }
    }
    else if (strcmp(args[1], hack_args[1]) != 0) {
      Serial.print("\nAttemping to hack ");
      Serial.print(args[1]);
      Serial.println(" fails. Try hacking a different way.\n");
    }
  }
  else if (location == "wall2") {
    if (strcmp(args[1], hack_args[3]) == 0) {
      Serial.println("\nHacking art is not for the faint of heart.\n");
      Serial.print("You enter '");
      Serial.print(args[2]);
      Serial.print("' with confidence...");
      delay(2000);
      if (strcmp(args[2], "john") == 0) {
        Serial.println("which is correct!!");
        Serial.println("John the Baptist is the person in the piece of piece of art.\n");
        solved2 = true;
      }
      else {
        Serial.println("...only to be disappointed.");
        Serial.println("\nOpen some glossy tomes to figure this one out.\n");
      }
    }
    else if (strcmp(args[1], hack_args[3]) != 0) {
      Serial.print("\nAttemping to hack ");
      Serial.print(args[1]);
      Serial.println(" fails. Try hacking a different way.\n");
    }
  }
  else if (location == "wall3") {
    if (strcmp(args[1], hack_args[2]) == 0) {
      rndIndex = random(25);
      hackcnt++;
      Serial.println("\nHacking the database, you eventually extract the following data:");
      Serial.print("     ");
      Serial.println(db_hashes[rndIndex]);
      Serial.println("\n");
      if (hackcnt > 8) {
        Serial.println("Sometimes, what you need is right under your nose.\n");
        solved3 = true;
      }
      //      else {
      //        Serial.println("This isn't rocket science. Try harder.\n");
      //      }
    }
    else if (strcmp(args[1], hack_args[2]) != 0) {
      Serial.println("This isn't rocket science. Try harder.\n");
      Serial.print("\nAttemping to hack ");
      Serial.print(args[1]);
      Serial.println(" fails. Try hacking a different way.\n");
    }
  }
  else if (location == "wall4") {
    if (strcmp(args[1], hack_args[5]) == 0) {
      Serial.println("\nAfter hours of laboriously trying different keys, the cryptex");
      Serial.println("gives a click! Daringly, you give the side a gentle tug...");
      delay(4000);
      if (strcmp(args[2], "apple") == 0) {
        Serial.println("\n...opens slightly. Pulling harder, a secret scroll is revealed!\n");
        Serial.println("You did it!! You opened the cryptex!!!\n");
        solved4 = true;
        hasScroll = true;
      }
      else {
        Serial.println("...only to be disappointed.\n");
        Serial.println("\nBe careful not to break the acidic vial inside. The secret will be");
        Serial.println("destroyed forever.\n");
      }
    }
    else if (strcmp(args[1], hack_args[5]) != 0) {
      Serial.print("\nAttemping to hack ");
      Serial.print(args[1]);
      Serial.println(" fails. Try hacking a different way.\n");
    }
  }
  else if (location == "wall5") {
    if (strcmp(args[1], hack_args[4]) == 0) {
      Serial.print("\nHacking the computer, you enter password '");
      Serial.print(args[2]);
      Serial.println("' with confidence...\n");
      if (strcmp(args[2], "esrwha") == 0) {
        Serial.println("The computer display goes blank...\n");
        tftBlank();
        delay(3500);
        tftBlank();
        Serial.println("What did you do?!\n");
        delay(1500);
        Serial.println("You entered the password correctly! That's what you did!\n");
        delay(2000);
        Serial.println("The computer unlocks to display a message.");
        delay(1000);
        Serial.println("             ____________________________________________________");
        Serial.println("            /                                                    \\ ");
        Serial.println("           |    _____________________________________________     |");
        Serial.println("           |   | To find the secrets of the cryptex,         |    |");
        Serial.println("           |   |  look where you can't see.                  |    |");
        Serial.println("           |   |                                             |    |");
        Serial.println("           |   | Follow where the bit. path takes you,       |    |");
        Serial.println("           |   |   assume you know nothing, be humble.ly     |    |");
        Serial.println("           |   |                                             |    |");
        Serial.println("           |   | Donate to Hak4Kidz https:\/\/paypal.me\/h4k    |    |");
        Serial.println("           |   | $>_                                         |    |");
        Serial.println("           |   |                                             |    |");
        Serial.println("           |   |_____________________________________________|    |");
        Serial.println("           |                                                      |");
        Serial.println("            \\_____________________________________________________/");
        Serial.println("                   \\_______________________________________/");
        Serial.println("                _______________________________________________");
        Serial.println("             _-'    .-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.  --- `-_");
        Serial.println("          _-'.-.-. .---.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.--.  .-.-.`-_");
        Serial.println("       _-'.-.-.-. .---.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-`__`. .-.-.-.`-_");
        Serial.println("    _-'.-.-.-.-. .-----.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-----. .-.-.-.-.`-_");
        Serial.println(" _-'.-.-.-.-.-. .---.-. .-----------------------------. .-.---. .---.-.-.-.`-_");
        Serial.println(":-----------------------------------------------------------------------------:");
        Serial.println("`---._.-----------------------------------------------------------------._.---'");
        Serial.println("\n");
        solved5 = true;
      }
      else {
        Serial.println("     but end up feeling confused.\n");
      }
    }
    else if (strcmp(args[1], hack_args[4]) != 0) {
      Serial.print("\nAttemping to hack ");
      Serial.print(args[1]);
      Serial.println(" fails. Try hacking a different way.\n");
    }
  }
  else {
    Serial.print("\nSorry, you can't hack the ");
    Serial.print(args[1]);
    Serial.println(" here.\n");
  }
}

int cmd_loc() {
  Serial.print("\nYour current location is: ");
  Serial.println(location);
}

// *********************
// ******  AREAS  ******
// *********************
//
// Center
// Wall 0 - Door
// Wall 1 - HHV
// Wall 2 - LV art
// Wall 3 - data flow
// Wall 4 - cryptex
// Wall 5 - computer

// ******************
// ***** CENTER *****
// ******************
void Center() {
  location = "center";
  //                      01234567890123456789012345678901234567890123456789012345678901234567890123456789
  Serial.println("You entered a six - sided room where you are standing in the center.");
  Serial.println("In front of each wall, a number from 0 thru 5 is painted in front of it.");
  Serial.println("Wall number 0 has a door, while the other walls have");
  Serial.println("various things for you to look at when approached.");
  Serial.println("Approach a wall by simply typing 'wall x' from the center of the room.");
  Serial.println("Where 'x' is the number of the wall.\n");
  Serial.println("");
  Serial.println("Type \"help\" for a list of commands or \"help intro\" to read the intro again.");
}

// ******************
// *****  DOOR  *****
// ******************
void Door() {
  //                01234567890123456789012345678901234567890123456789012345678901234567890123456789
  location = "door";
  if (solved0) {
    Serial.println("\nFerris Bueller appears wearing a gray and maroon striped bath robe with a");
    Serial.println("perplexed look and says,");
    Serial.println("\"You're still here? It's over.\"");
    Serial.println("\"Go home...Go.\"");
    Serial.println("Ferris leaves through the door.\n");
    cmd_exit();
  }
  else {
    Serial.println("             +------------------------------+");
    Serial.println("             |                              |");
    Serial.println("             |                              |");
    Serial.println("             |                              |");
    Serial.println("             |                              |");
    Serial.println("             |                              |");
    Serial.println("             |                              |");
    Serial.println("             |                              |");
    Serial.println("             |                              |    +-------+");
    Serial.println("             |                              |    | 1 2 3 |");
    Serial.println("             |                              |    | 4 5 6 |");
    Serial.println("             |                         _.., |    | 7 8 9 |");
    Serial.println("             |                              |    |   0   |");
    Serial.println("             |                              |    +-------+");
    Serial.println("             |                              |");
    Serial.println("             |                              |");
    Serial.println("             |                              |");
    Serial.println("             |                              |");
    Serial.println("             |                              |");
    Serial.println("             |                              |");
    Serial.println("             |                              |");
    Serial.println("             |                              |");
    Serial.println("             bbbbbbbbbbbbbbbb8PPPPPPPPPPPPPPP");
    Serial.println("");
    Serial.println("\nApproaching wall 0, the door in the wall is visually clearer. You notice on the");
    Serial.println("right side of the door a numeric keypad from 0-9 like an old timey push button ");
    Serial.println("phone. What code do you want to try to unlock the door?\n");
  }
}

void Wall1() {
  location = "wall1";
  //            01234567890123456789012345678901234567890123456789012345678901234567890123456789
  if (solved1) {
    // The puzzle is solved
    Serial.println("\nBored? Back again to make something else I hope.");
    Serial.println("You already entered the code, so don't try it again.");
    Serial.println(" Stranger things may happens...\n");
  } else {
    // A puzzle to do
    Serial.println("\nApproaching wall 1, you feel this compelling sensation to solder something.");
    Serial.println("This wall only has a mirror hanging, with your confused looking reflection.");
    Serial.println("Go to the HHV, or find a soldering station and make something functional.");
    Serial.println("When you show it to the correct person, you'll receive the solution.");
    Serial.println("Funny how you have to go someplace, to do something to escape from this room.\n");
  }
}

void Wall2() {
  location = "wall2";
  if (solved2) {
    // The puzzle is solved
    Serial.println("\n Welcome back! A fellow art lover. Take your time and enjoy it.\n");
  } else {
    // A puzzle to do
    Serial.println("\nWall 2 has a piece of a piece of art painted on it. Perhaps by LV himself?");
    Serial.println("Vist https\/\/:bit.ly\/2Ybyf9K to see who this secret symbol represents.\n");
  }
}

void Wall3() {
  location = "wall3";
  if (solved3) {
    // The puzzle is solved
    Serial.println("\nGiving the data flow another good hack again?\nWell, hack away my friend! Hack away!");
    Serial.println("There's plenty of hashes in there.\n");
  } else {
    // A puzzle to do
    Serial.println("\nApproaching wall 3, from a distance, what looked like water flowing down the wall isn't water...\n");
    Serial.println("As you move closer, what you believed to be water is actually flowing data.");
    Serial.println("The wall is a full sized display of masked information. To the right is a small sign that reads,");
    Serial.println("\n                              'HACK ME'\n");
    Serial.println("Well? Do you?\n");
  }
}

void Wall4() {
  location = "wall4";
  if (solved4) {
    // The puzzle is solved
    Serial.println("\nNice work solving this puzzle already! Have you found the true secret yet?");
    Serial.println("I guess not because why else would you come back.\n");
  } else {
    // A puzzle to do
    Serial.println("\nAs you approach wall 4, there is a console table against the wall.");
    Serial.println("Nicely placed on the table is a sealed cylinder. The cylinder beckons");
    Serial.println("you to look at is more closely.\n");
  }
}

void Wall5() {
  location = "wall5";
  if (solved5) {
    // The puzzle is solved
    Serial.println("\nYou already solved this. Move on to the next stage.");
    Serial.println("Rumor has it that there is a prize waiting...\n");
  } else {
    // A puzzle to do
    Serial.println("\nAs you approach wall 5, before you is a wooden workstation with");
    Serial.println("a computer terminal resting upon it.\n");
  }
}

int cmd_wall() {
  if (location == "center") {
    if (atoi(args[1]) == 0) {
      location = "door";
      Door();
      // Serial.print("\nPlease only use numbers to approach the walls.\n");
    }
    else if (atoi(args[1]) == 1) {
      location = "wall1";
      Wall1();
    }
    else if (atoi(args[1]) == 2) {
      location = "wall2";
      Wall2();
    }
    else if (atoi(args[1]) == 3) {
      location = "wall3";
      Wall3();
    }
    else if (atoi(args[1]) == 4) {
      location = "wall4";
      Wall4();
    }
    else if (atoi(args[1]) == 5) {
      location = "wall5";
      Wall5();
    }
    else if (location != "center") {
      Serial.println("\nReturn to room center to approach another wall.\n");
    }
  }
  else {
    Serial.println("\nImproper use of WALL command. Try \"help wall\" for advice.\n");
  }
}

int cmd_cntr() {
  if (location != "center") {
    Serial.println("\nReturning to center.");
    Center();
  }
  else {
    Serial.println("Wait..what? You're already in the center of the room. Go towards a wall.");
  }
}

// *********************
// ******  FLUFFY ******
// ******  STUFF  ******
// *********************
//

void credits() {
  //              01234567890123456789012345678901234567890123456789012345678901234567890123456789
  Serial.println("\n\n");
  Serial.println("        ██╗  ██╗ █████╗ ██╗  ██╗██╗  ██╗██╗  ██╗██╗██████╗ ███████╗");
  Serial.println("        ██║  ██║██╔══██╗██║ ██╔╝██║  ██║██║ ██╔╝██║██╔══██╗╚══███╔╝");
  Serial.println("        ███████║███████║█████╔╝ ███████║█████╔╝ ██║██║  ██║  ███╔╝");
  Serial.println("        ██╔══██║██╔══██║██╔═██╗ ╚════██║██╔═██╗ ██║██║  ██║ ███╔╝ ");
  Serial.println("        ██║  ██║██║  ██║██║  ██╗     ██║██║  ██╗██║██████╔╝███████╗");
  Serial.println("        ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝     ╚═╝╚═╝  ╚═╝╚═╝╚═════╝ ╚══════╝");
  Serial.println("");
  Serial.println("         ██████╗██████╗ ██╗   ██╗██████╗ ████████╗███████╗██╗  ██╗ ");
  Serial.println("        ██╔════╝██╔══██╗╚██╗ ██╔╝██╔══██╗╚══██╔══╝██╔════╝╚██╗██╔╝");
  Serial.println("        ██║     ██████╔╝ ╚████╔╝ ██████╔╝   ██║   █████╗   ╚███╔╝ ");
  Serial.println("        ██║     ██╔══██╗  ╚██╔╝  ██╔═══╝    ██║   ██╔══╝   ██╔██╗  ");
  Serial.println("        ╚██████╗██║  ██║   ██║   ██║        ██║   ███████╗██╔╝ ██╗ ");
  Serial.println("         ╚═════╝╚═╝  ╚═╝   ╚═╝   ╚═╝        ╚═╝   ╚══════╝╚═╝  ╚═╝ ");
  Serial.println("");
  delay (2000);
  Serial.println("\n\n\n\n\n\n\n\n\n\n");
  Serial.println("                           Hak4Kidz Virtual Escape Room");
  Serial.println("");
  Serial.println("                        A virtual badge escape adventure");
  Serial.println("                        Designed and written by W K Jones");
  Serial.println("                                 Adapted by Heal");
  Serial.println("                                 www.hak4kidz.com\n\n");
  Serial.println("                       Follow @Hak4Kidz for badge updates.");
  Serial.println("\n\n");
}

void intro() {
  //                01234567890123456789012345678901234567890123456789012345678901234567890123456789
  Serial.println("POOF!!!");
  Serial.println("You are trapped in a six-sided room, standing in the center.");
  Serial.println("Each wall has a number from 0 thru 5 painted on the floor in front of it.");
  Serial.println("Wall number 0 has a door, while the other walls have various things for you to");
  Serial.println("look at when approached. Approach a wall by simply typing \"wall x\" where X is");
  Serial.println("the wall number from the center of the room. Return to center after each wall\n");
  Serial.println("Type \"help\" for a list of commands or \"help intro\" to read this intro again.\n");
}

void cli_init() {
  //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
  Serial.print("\n\n\n\nver. ");
  Serial.print(badge_ver);
  Serial.println("\n");
  Serial.println("Welcome to this simple cryptex command line interface (CLI).\n");
  Serial.println("Be sure to type quickly, the CLI doesn't have patience. It will press the");
  Serial.println("ENTER key for you. Use a trailing space before pressing ENTER yourself.\n");
  delay (2000);

  credits();
  intro();

}

int cmd_intro() {
  cli_init();
}

void badge_cli() {

  Serial.print("Ready> ");

  read_line();
  if (!error_flag) {
    parse_line();
  }
  if (!error_flag) {
    execute();
  }

  memset(line, 0, LINE_BUF_SIZE);
  memset(args, 0, sizeof(args[0][0]) * MAX_NUM_ARGS * ARG_BUF_SIZE);

  error_flag = false;
}
