#if DEBUG
#define TempFile
#endif

#define CELESTE

using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using System.Threading;
using System.Drawing;

namespace GBA_Complier {

    struct FamiTrackerRow {
        public int note, octave, volume;

        public FamiTrackerRow(int _note, int _octave, int _volume) {
            note = _note;
            octave = _octave;
            volume = _volume;
        }

        public static FamiTrackerRow DEFAULT {
            get {
                FamiTrackerRow retval = new FamiTrackerRow(0, 0, 0x10);

                return retval;
            }
        }
    }
    class FileEditedTracker {
        struct EditData {
            public string name;
            public long lastEditTick;

            public EditData(string _line) {
                string[] split = _line.Split(';');
                name = split[0];
                lastEditTick = long.Parse(split[1]);
            }
            public EditData(string _name, long _tick) {
                name = _name;
                lastEditTick = _tick;
            }

            public override string ToString() {
                return name + ";" + lastEditTick.ToString();
            }
        }

        List<EditData> edit = new List<EditData>();
        List<string> fileContents;
        string path, directory, searchType;
        bool fullSearch;

        public FileEditedTracker(string _directory, string _path, string _search = ".*", bool _fullSearch = false) {
            path = _path;
            directory = _directory;
            searchType = _search;
            fullSearch = _fullSearch;

            if (!File.Exists(_path)) {
                File.Create(_path).Close();
                fileContents = new List<string>();
            }
            else {

                fileContents = new List<string>(File.ReadAllLines(_path));

                foreach (var s in fileContents)
                    edit.Add(new EditData(s));
            }
        }

        private void EditChangeTime(string _name, long _tick) {
            string[] split;
            for (int i = 0; i < fileContents.Count; ++i) {
                split = fileContents[i].Split(';');
                if (split[0] == _name) {
                    fileContents[i] = split[0] + ";" + _tick.ToString();
                    File.WriteAllLines(path, fileContents.ToArray());
                    break;
                }
            }
        }

        public bool AnyFileChanged() {
            bool retVal = false;

            foreach (string s in Directory.GetFiles(directory, searchType, fullSearch ? SearchOption.AllDirectories : SearchOption.TopDirectoryOnly)) {
                if (FileNeedsUpdate(s.Replace(directory, "")))
                    retVal = true;
            }
            return retVal;
        }
        public bool FileNeedsUpdate(string _fileName) {

            //return true;

            string filePath = directory + "/" + _fileName;
            if (!File.Exists(filePath))
                throw new Exception();
            long tick = File.GetLastWriteTime(filePath).Ticks;

            foreach (var f in edit) {
                if (f.name == _fileName) {
                    if (f.lastEditTick < tick) {
                        EditChangeTime(_fileName, tick);
                        return true;
                    }
                    else if (f.lastEditTick == tick) {

                    }
                    return false;
                }
            }

            EditData data;
            edit.Add(data = new EditData(_fileName, tick));
            fileContents.Add(data.ToString());

            File.WriteAllLines(path, fileContents.ToArray());

            return true;
        }
    }
    struct DialogueInfo {
        public byte[] bytes;
        public string name;
    }
    public class VisualData {
        public byte[,] FG, BG;
        public char[,] InfoFG, InfoBG;

        public void SetByte(int _x, int _y, bool _fg, byte _data) {
            (_fg ? FG : BG)[_x, _y] = _data;
        }
        public byte GetByte(int _x, int _y, bool _fg) {
            if (_x < 0)
                _x = 0;
            if (_x >= FG.GetLength(0))
                _x = FG.GetLength(0) - 1;
            if (_y < 0)
                _y = 0;
            if (_y >= FG.GetLength(1))
                _y = FG.GetLength(1) - 1;

            return (_fg ? FG : BG)[_x, _y];
        }
        public void SetData(int _line, int _w, int _h, string[] _data) {
            InfoFG = new char[_w, _h];
            InfoBG = new char[_w, _h];
            FG = new byte[_w, _h];
            BG = new byte[_w, _h];

            var hasBG = _data.Length > _line + _h && _data[_line + _h].ToLower().StartsWith("bg");

            for (int y = 0; y < _h; ++y) {
                for (int x = 0; x < _w; ++x) {
                    InfoFG[x, y] = _data[_line + y][x];
                    if (hasBG)
                        InfoBG[x, y] = _data[_line + y + _h + 1][x];
                }
            }
        }
        public char GetData(int _x, int _y, bool _fg) {

            return (_fg ? InfoFG : InfoBG)[_x, _y];
        }
    }
    class Program {
        
        static Dictionary<char, ushort> decoration;

        //Visual byte data
        const byte V_SOLID_1 = 1, V_SOLID_2 = 2, V_SOLID_3 = 3, V_SPIKEUP = 4, V_SPIKEDOWN = 5, V_SPIKERIGHT = 6, V_SPIKELEFT = 7, V_DREAMBLOCK = 8, V_SPINNER = 9, V_PLATFORM = 10, V_NOTE1 = 11, V_NOTE2 = 12, V_WATER = 13, V_PLAYER = 0xF, V_SOLIDBG_1 = 0x11, V_SOLIDBG_2 = 0x12, V_SOLIDBG_3 = 0x13;

        const byte C_SOLID = 1, C_DREAM = 2, C_PLATFORM = 3, C_SPIKE_U = 4, C_SPIKE_D = 5, C_SPIKE_L = 6, C_SPIKE_R = 7, C_SPINNER = 8, C_NOTE1 = 9, C_NOTE2 = 10, C_STRAWB = 11, C_WATER = 12;

        const byte ARROW_INDEX = 41;

#if CELESTE
        static bool DoubleCollision = false;
#else
        static bool DoubleCollision = true;
#endif
        const int FRAMES_PER_QUARTERNOTE = 16;
        static readonly Dictionary<string, int> notes = new Dictionary<string, int>();
        static Random randomizer = new Random();

        static Dictionary<string, byte> portaitNames = new Dictionary<string, byte>()
        {
            {"madeline", 0},
            {"badeline", 1},
        };

        static List<string> rooms;
        static int LevelDataSize = 0, ArtDataSize = 0, StrawbCount, StrawbMax;
        const int ARRAY_WIDTH_USHORT = 8;
        const int ARRAY_WIDTH = 16;
        static string[] intro = new string[] {
            "set path=C:\\devkitadv-r5-beta-3\\bin;%path%",
            "del output\\celeste.gba"
        };

        public static string path;

        static VisualData ground = new VisualData();
        static int width, height;

        static void Main(string[] args) {
            notes.Add("C", 8013);
            notes.Add("C#", 7566);
            notes.Add("Db", 7566);
            notes.Add("D", 7144);
            notes.Add("D#", 6742);
            notes.Add("Eb", 6742);
            notes.Add("E", 6362);
            notes.Add("F", 6005);
            notes.Add("F#", 5666);
            notes.Add("Gb", 5666);
            notes.Add("G", 5346);
            notes.Add("G#", 5048);
            notes.Add("Ab", 5048);
            notes.Add("A", 4766);
            notes.Add("A#", 4499);
            notes.Add("Bb", 4499);
            notes.Add("B", 4246);

#if TempFile
            path = "C:/Users/IsaGoodFriend/OneDrive/Documents/DevKitPro/Projects/Celeste/";
#else
            path = Directory.GetCurrentDirectory();
#endif
            if (path.EndsWith("\\") || path.EndsWith("/")) {
                path = path.Remove(path.Length - 1);
            }

            FileEditedTracker tracker = new FileEditedTracker(path + "/levels", path + "/editedLvl.txt", "*.txt", true);
            
            if (tracker.AnyFileChanged() || args.Contains("-clean") || args.Contains("-rebuild")) {
                if (CompileLevels() != 0) {
                    return;
                }
            }

            tracker = new FileEditedTracker(path + "/sprites", path + "/editedSprite.txt", "*.bmp");
            if (tracker.AnyFileChanged() || args.Contains("-clean") || args.Contains("-rebuild"))
                CompileSprites();

            tracker = new FileEditedTracker(path + "/dialogue", path + "/editedDlg.txt", "*.txt");
            if (tracker.AnyFileChanged() || args.Contains("-clean") || args.Contains("-rebuild"))
                CompileText();

            tracker = new FileEditedTracker(path + "/transitions", path + "/editedTrans.txt", "*.bmp");
            if (tracker.AnyFileChanged() || args.Contains("-clean") || args.Contains("-rebuild"))
                CompileTransitions();

            tracker = new FileEditedTracker(path + "/sprites", path + "/editedBG.txt", "BG_Maps.raw", true);
            //if (tracker.AnyFileChanged() || args.Contains("-clean") || args.Contains("-rebuild"))
                CompileBackgrounds();


            tracker = new FileEditedTracker(path + "/gb music", path + "/edited8bit.txt", "*.ftm", true);
            if (tracker.AnyFileChanged() || args.Contains("-clean") || args.Contains("-rebuild"))
                CompileSongs();

            Process cmd = new Process();
            ProcessStartInfo info = new ProcessStartInfo();
            info.FileName = "cmd.exe";
            info.RedirectStandardInput = true;
            info.UseShellExecute = false;

            cmd.StartInfo = info;
            cmd.Start();

            using (StreamWriter sw = cmd.StandardInput) {
                if (sw.BaseStream.CanWrite) {
                    sw.WriteLine("cd " + path);

                    if (args.Contains("-clean"))
                        sw.WriteLine("make clean");
                    else
                        sw.WriteLine("make");

                    if (args.Contains("-run"))
                        sw.WriteLine("celeste.gba");
                }
            }

            cmd.WaitForExit();

            Console.WriteLine("Finished");

            Console.WriteLine("Art data size: " + (ArtDataSize / 1024.0f).ToString() + " kilobytes");
            Console.WriteLine("Level data size: " + (LevelDataSize / 1024.0f).ToString() + " kilobytes");

#if TempFile
            Console.ReadKey();

#endif

        }

        //get the list of rooms in the level.
        // compile width and height, then the four entrances for each room (either by entering direction, or by position.  unsure yet)
        // compile the indexes of each exit's next room (up, down, left, right)
        // compile the foreground/background

        /// <summary>
        /// find each file in the level folder (that's not array.txt)
        /// read each file's content.
        /// compile that into a list of bytes
        /// compile the list of bytes into the .c and .h file
        /// </summary>
        static int CompileLevels() {
            if (!Directory.Exists(path + "/levels"))
                return 0;

            int errors = 0;
            List<string> output_C_last = new List<string>();
            List<string> output_C = new List<string>();
            List<string> output_H = new List<string>();

            string[] getFiles = Directory.GetFiles(path + "/levels");

            int levelCount;

            foreach (string s in getFiles) {
                if (!s.EndsWith(".txt"))
                    continue;

                string[] split = s.Split('\\');
                string name = Path.GetFileNameWithoutExtension(s);

                if (name.Contains("levels_")) {
                    string[] cont = File.ReadAllLines(s);

                    string levelName = cont[0].ToLower();
                    if (levelName.Length != 16)
                        throw new Exception();

                    decoration = new Dictionary<char, ushort>();

                    int offset = 1;
                    ushort decoOff = 0x4080;

                    for (; cont[offset] != "levels:"; ++offset) {
                        
                        if (!string.IsNullOrWhiteSpace(cont[offset])) {
                            split = cont[offset].Split(':');

                            switch (split[0]){
                                case "off":
                                    decoOff = (ushort)((decoOff & 0xF000) + (int.Parse(split[1])));
                                    break;
                                case "pal":
                                    decoOff = (ushort)((decoOff & 0x1FF) + ((int.Parse(split[1]) + 4) << 12));
                                    break;
                                default:
                                    ushort value = (ushort)(GetDecoValue(split) + decoOff);
                                    decoration.Add(cont[offset][0], value);

                                    break;
                            }
                        }
                    }

                    levelCount = 0;
                    for (int i = offset; i < cont.Length; ++i) {
                        if (cont[i] != "levels:")
                            continue;
                        do {
                            ++i;
                            ++levelCount;
                        }
                        while (i < cont.Length && !string.IsNullOrEmpty(cont[i]));
                        --levelCount;
                    }

                    output_H.Add("#define MAX_" + name + " " + levelCount);
                    output_H.Add("extern unsigned char* " + name + "[" + levelCount + "];");

                    output_C_last.Add("unsigned char* " + name + "[" + levelCount + "]=");
                    output_C_last.Add("{");

                    name = name.Remove(0, 7);

                    ++offset;

                    rooms = new List<string>();
                    for (int index = offset; index < cont.Length; ++index) {

                        rooms.Add(cont[index]);
                        output_C_last.Add("\t(unsigned char*)&" + name + "_" + cont[index] + ",");
                        
                    }
                    output_C_last.Add("};");

                    output_H.Add("extern const char " + name + "_visName[16];");

                    output_C_last.Add("const char " + name + "_visName[16] = \"" + levelName + "\";");

                    CompileLevelRooms(name, output_C, output_H);
                }
            }

            if (errors == 0) {
                output_C.AddRange(output_C_last);
                File.WriteAllLines(path + "/source/level.c", output_C.ToArray());
                File.WriteAllLines(path + "/source/level.h", output_H.ToArray());
            }

            return errors;
        }
        static void CompileText() {
            List<string> output_C = new List<string>(), output_CLast = new List<string>(), output_H = new List<string>();

            foreach (var paths in Directory.GetFiles(path + "/dialogue")) {
                string name = Path.GetFileNameWithoutExtension(paths);

                List<string> textBoxes = new List<string>();
                string boxName;
                int index = 0;
                foreach (var data in CompileText(paths)) {
                    boxName = name + "_" + index++;
                    textBoxes.Add(boxName);
                    WriteByteArray(output_C, output_H, data.bytes, boxName);
                }

                output_H.Add("#define boxCount_" + name + " " + textBoxes.Count);
                output_H.Add("extern unsigned char* " + name + "[" + textBoxes.Count + "];");

                output_CLast.Add("unsigned char* " + name + "[" + textBoxes.Count + "]=");
                output_CLast.Add("{");

                foreach (string str in textBoxes) {
                    output_CLast.Add("\t(unsigned char*)&" + str + ",");
                }
                output_CLast.Add("};");
            }
            output_C.AddRange(output_CLast);


            File.WriteAllLines(path + "/source/text.c", output_C.ToArray());
            File.WriteAllLines(path + "/source/text.h", output_H.ToArray());
        }
        static IEnumerable<DialogueInfo> CompileText(string _file) {
            string[] fileLines = File.ReadAllLines(_file);

            DialogueInfo info = new DialogueInfo();
            List<byte> textData = new List<byte>();

            for (int i = 0; i < fileLines.Length; ++i) {
                textData.Clear();

                // Wait until found more dialogue
                while (!fileLines[i].StartsWith("[") || !fileLines[i].EndsWith("]"))
                    ++i;

                // Get dialogue data
                string[] data = fileLines[i].Replace("[", "").Replace("]", "").Split(':');
                ++i;

                // Get dialogue name
                info.name = data[0];

                // Get portrait info
                textData.Add(portaitNames[data[1]]);
                textData.Add(byte.Parse(data[2]));

                for (int j = 0; j < 4; ++j) {
                    if (fileLines[i] == "end") {
                        do {
                            textData.AddRange(new byte[23] { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, (byte)(j == 3 ? ARROW_INDEX : 0xFF) });

                        } while (++j < 4);

                        break;
                    }

                    for (int ch = 0; ch < 23; ++ch) {
                        if (ch == 22 && j == 3)
                            textData.Add(ARROW_INDEX);
                        else if (ch < fileLines[i].Length)
                            textData.Add(Char2TextData(fileLines[i][ch], fileLines[i][ch == 0 ? ch : ch - 1]));
                        else
                            textData.Add(0xFF);
                    }

                    ++i;
                }

                info.bytes = textData.ToArray();

                yield return info;
            }

            yield break;
        }

        static void FlipRoom(string _path, string[] data) {
            string oldPath = _path;

            _path = _path.Replace("_flip", "");

            string[] split = data[0].Split(';');
            int width = int.Parse(split[0]);
            int height = int.Parse(split[1]);

            for (int i = 1; i <= 4; ++i) {
                split = data[i].Split(';');

                data[i] = (width - int.Parse(split[0])).ToString("D2") + ';' + split[1];
            }

            split = data[5].Split(';');
            data[5] = split[0] + ';' + split[1] + ';' + split[3] + ';' + split[2];

            int index = 6;
            for (int i = 0; i < height; ++i) {
                char[] v = data[index].ToCharArray();
                Array.Reverse(v);
                string val = new string(v);
                val = val.Replace(',', 'X').Replace('.', ',').Replace('X', '.');

                data[index] = val;

                ++index;
            }
            if (data.Length > index && data[index] == "bg") {
                ++index;
                for (int i = 0; i < height; ++i) {
                    char[] v = data[index].ToCharArray();
                    Array.Reverse(v);
                    data[index] = new string(v);

                    ++index;
                }
            }
            for (; index < data.Length; ++index) {
                split = data[index].Split(';');
                string s = split[0];

                split[1] = (width - int.Parse(split[1])).ToString();

                for (int i = 1; i < split.Length; ++i)
                    s += ";" + split[i];

                data[index] = s;
            }

            File.Delete(oldPath);
            File.WriteAllLines(_path, data);
        }

        const int TRANSITION_FRAME_COUNT = 16;
        static void CompileTransitions() {
            List<string> output_C = new List<string>();
            List<string> output_H = new List<string>();

            string[] files = Directory.GetFiles(path + "/transitions");

            foreach (string s in files) {
                if (Path.GetExtension(s) != ".bmp") continue;

                Bitmap map = new Bitmap(s);
                if (map.Height < TRANSITION_FRAME_COUNT * 160 || map.Width != 240)
                    throw new Exception("Image file " + s + "is not of the correct size");

                List<ushort> win0Vals = new List<ushort>(), win1Vals = new List<ushort>();

                for (int line = 0; line < 160 * TRANSITION_FRAME_COUNT; ++line) {
                    int left0 = 0, right0 = 0, left1 = 0, right1 = 0;
                    Color c;

                    while (left0 < 240 && (c = map.GetPixel(left0, line)).R == 0) {
                        ++left0;
                    }
                    right0 = left0;
                    while (right0 < 240 && map.GetPixel(right0, line).R > 0) {
                        ++right0;
                    }

                    left0 &= 0xFF;
                    right0 &= 0xFF;
                    left0 <<= 8;
                    win0Vals.Add((ushort)(left0 | right0));

                    left1 &= 0xFF;
                    right1 &= 0xFF;
                    left1 <<= 8;
                    win1Vals.Add((ushort)(left1 | right1));
                }

                WriteUshortArray(output_C, output_H, win0Vals, Path.GetFileNameWithoutExtension(s) + "_trans0");
                //WriteUshortArray(output_C, output_H, win1Vals, Path.GetFileNameWithoutExtension(s) + "_trans1");
            }

            File.WriteAllLines(path + "/source/transitions.c", output_C.ToArray());
            File.WriteAllLines(path + "/source/transitions.h", output_H.ToArray());
        }
        static int CompileSongs() {
            List<string> output_C = new List<string>();
            List<string> output_H = new List<string>();

            output_H.Add("#ifndef __SONGS__\n#define __SONGS__\n");
            output_C.Add("#include \"songs.h\"\n");

            string[] getFiles = Directory.GetFiles(path + "/gb music");

            foreach (string s in getFiles) {
                string name = Path.GetFileNameWithoutExtension(s);
                string ext = Path.GetExtension(s);

                List<ushort> dataSQ1 = new List<ushort>(), dataSQ2 = new List<ushort>(), dataSQ3 = new List<ushort>();

                string str = null;
                if (ext == ".txt") {
                    #region TEXT MUSIC
                    string[] content = File.ReadAllLines(s);

                    //pseudo:
                    // read each line.  detect which voice it runs.
                    // Get the volume of the line.
                    // foreach note.
                    // detect which note and octave it is.  Get the length of the note, multiply it to get the frames.
                    // add note data to the list
                    int voice = 0;
                    int volume, crescendo = 0;
                    int pitch, noteLength, mode = 2;
                    List<string> splitNote;
                    for (int i = 0; i < content.Length; ++i) {
                        str = content[i];

                        // If string isn't note commands, or is empty, continue
                        if (str.Length == 0 || !char.IsDigit(str[0]))
                            continue;

                        // Remove whitespace
                        str = str.Replace(" ", "").Replace("\t", "");

                        string[] splitMain = str.Split(';');
                        voice = int.Parse(splitMain[0]);

                        volume = int.Parse(splitMain[1]);
                        if (splitMain[0] != "3") {
                            crescendo = int.Parse(splitMain[2]);

                            mode = int.Parse(splitMain[3]);
                        }

                        for (int j = (splitMain[0] != "3") ? 4 : 2; j < splitMain.Length; ++j) {
                            splitNote = new List<string>(splitMain[j].Split('/', '\\', '|'));
                            int tempVolume = volume;
                            int crescendoTemp = crescendo;
                            int sweep = 0;
                            bool staccato = false;

                            if (splitNote[0] == "R") {
                                pitch = 0;
                                noteLength = (int)(float.Parse(splitNote[1]) * FRAMES_PER_QUARTERNOTE);
                                tempVolume = 0;
                                crescendoTemp = 0;
                            }
                            else {
                                pitch = GetPitch(int.Parse(splitNote[1]), splitNote[0]);
                                noteLength = (int)(float.Parse(splitNote[2]) * FRAMES_PER_QUARTERNOTE);

                                for (int k = 3; k < splitNote.Count; ++k) {
                                    if (splitNote[k].Contains("vol:")) {
                                        tempVolume = int.Parse(splitNote[k].Substring(4));
                                    }
                                    else if (splitNote[k].Contains("pb:")) {
                                        pitch += int.Parse(splitNote[k].Substring(3));
                                    }
                                    else if (splitNote[k].Contains("cresc:")) {
                                        crescendoTemp += int.Parse(splitNote[k].Substring(6));
                                    }
                                    else if (splitNote[k] == "st") {
                                        staccato = true;
                                    }
                                }
                            }

                            if (splitMain[0] == "1") {
                                if (staccato) {
                                    AddNoteSQ1(dataSQ1, pitch, tempVolume, noteLength >> 1, sweep, 0, crescendoTemp, mode);
                                    AddNoteSQ1(dataSQ1, 0, 0, noteLength - (noteLength >> 1), 0, 0, 0, mode);
                                }
                                else {
                                    AddNoteSQ1(dataSQ1, pitch, tempVolume, noteLength, sweep, 0, crescendoTemp, mode);
                                }
                            }
                            else if (splitMain[0] == "2") {
                                AddNoteSQ2(dataSQ2, pitch, tempVolume, noteLength, crescendoTemp, mode);
                            }
                            else if (splitMain[0] == "3") {
                                if (staccato) {
                                    AddNoteSQ3(dataSQ3, pitch, tempVolume, noteLength >> 1);
                                    AddNoteSQ3(dataSQ3, 0, 0, noteLength - (noteLength >> 1));
                                }
                                else {
                                    AddNoteSQ3(dataSQ3, pitch, tempVolume, noteLength);
                                }
                            }
                        }
                    }
                    #endregion
                }
                else if (ext == ".ftm") {
                    #region FAMITRACKER MODULE

                    byte[] data = File.ReadAllBytes(s);

                    int FRAMES_header = 0, PATTERNS_header = 0;

                    for (int i = 0x300; i < data.Length - 16 && (FRAMES_header == 0 || PATTERNS_header == 0); ++i) {
                        string hName = Encoding.Default.GetString(data, i, 16);

                        if (hName.StartsWith("FRAMES")) {
                            i += 20;
                            FRAMES_header = i;
                            byte test = data[i + 4];
                        }
                        if (hName.StartsWith("PATTERNS")) {
                            i += 20;
                            PATTERNS_header = i;
                        }
                    }
                    int frameCount = BitConverter.ToInt32(data, FRAMES_header + 0x04),
                        rowPerFrame = BitConverter.ToInt32(data, FRAMES_header + 0x10);
                    int tempo = BitConverter.ToInt32(data, FRAMES_header + 0x0C);
                    float ratio = tempo / 110.0f;
                    tempo = (int)(8 / ratio);

                    int patternDataSize = BitConverter.ToInt32(data, PATTERNS_header);
                    PATTERNS_header += 4;

                    byte[,] patterns = new byte[3, frameCount];
                    for (int i = 0; i < 5 * frameCount; ++i) {
                        if (i % 5 >= 3)
                            continue;

                        patterns[i % 5, i / 5] = data[FRAMES_header + 0x14 + i];
                    }

                    List<FamiTrackerRow[]>[] songInfo = new List<FamiTrackerRow[]>[] { new List<FamiTrackerRow[]>(), new List<FamiTrackerRow[]>(), new List<FamiTrackerRow[]>() };

                    for (int i = 0; i < patternDataSize;) {
                        int channel = BitConverter.ToInt32(data, PATTERNS_header + i + 0x04);
                        int pattern = BitConverter.ToInt32(data, PATTERNS_header + i + 0x08);
                        int rows =    BitConverter.ToInt32(data, PATTERNS_header + i + 0x0C);
                        

                        i += 0x10;

                        FamiTrackerRow[] rowData = null;
                        while (channel <= 2 && songInfo[channel].Count <= pattern) {
                            songInfo[channel].Add(rowData = new FamiTrackerRow[rowPerFrame]);
                            for (int j = 0; j < rowPerFrame; ++j) {
                                rowData[j] = FamiTrackerRow.DEFAULT;
                            }
                        }
                        
                        if (channel <= 2)
                            rowData = songInfo[channel][pattern];

                        for (int j = 0; j < rows; ++j) {
                            int index = BitConverter.ToInt32(data, PATTERNS_header + i);
                            if (index >= rowPerFrame) {
                                i += 0x0A;// 0x08 + 0x02 (effect)
                                continue;
                            }

                            if (channel <= 2)
                                rowData[index] = new FamiTrackerRow(data[PATTERNS_header + i + 0x4], data[PATTERNS_header + i + 0x5], data[PATTERNS_header + i + 0x7]);

                            i += 0x0A;// 0x08 + 0x02 (effect)
                        }
                    }

                    for (int channel = 0; channel < 3; ++channel) {
                        int noteLength = 0;
                        int volume = -1;
                        int pitch = 0;
                        int pattern = 0;

                        for (int row = 0; row < rowPerFrame * frameCount; ++row) {

                            if (row % rowPerFrame == 0) {
                                pattern = patterns[channel, row / rowPerFrame];
                            }

                            var note = songInfo[channel][pattern][row % rowPerFrame];
                            if (note.note == 0x00) {
                                ++noteLength;
                                continue;
                            }

                            int newV = note.volume;
                            int newP = GetPitch(note.octave, note.note - 1);
                            
                            if (volume != newV || pitch != newP) {
                                if (volume != -1) {

                                    noteLength = noteLength * tempo;
                                    switch (channel) {
                                        case 0:
                                            AddNoteSQ1(dataSQ1, pitch, volume, noteLength, 0, 0, 7, 1);
                                            break;
                                        case 1:
                                            AddNoteSQ2(dataSQ2, pitch, volume, noteLength, 7, 1);
                                            break;
                                        case 2:
                                            AddNoteSQ3(dataSQ3, pitch, volume >> 2, 4);
                                            AddNoteSQ3(dataSQ3, 0, 0, noteLength - 4);
                                            break;
                                    }
                                }

                                if (newV != 0x10)
                                    volume = newV;
                                pitch = newP;
                                noteLength = 0;
                            }
                            ++noteLength;
                        }

                        noteLength = (noteLength * FRAMES_PER_QUARTERNOTE) >> 1;

                        switch (channel) {
                            case 0:
                                AddNoteSQ1(dataSQ1, pitch, volume, noteLength, 0, 0, 7, 1);
                                break;
                            case 1:
                                AddNoteSQ2(dataSQ2, pitch, volume, noteLength, 7, 1);
                                break;
                            case 2:
                                AddNoteSQ3(dataSQ3, pitch, volume >> 2, 4);
                                AddNoteSQ3(dataSQ3, 0, 0, noteLength - 4);
                                break;
                        }
                    }

                    #endregion
                }
                else {
                    continue;
                }

                WriteUshortArray(output_C, output_H, dataSQ1, name + "_song_sq1");
                WriteUshortArray(output_C, output_H, dataSQ2, name + "_song_sq2");
                WriteUshortArray(output_C, output_H, dataSQ3, name + "_song_sq3");
            }


            output_H.Add("#endif");

            File.WriteAllLines(path + "/source/songs.c", output_C.ToArray());
            File.WriteAllLines(path + "/source/songs.h", output_H.ToArray());

            return 0;
        }
        static int CompileLevelRooms(string folderName, List<string> output_C, List<string> output_H) {
            string pathHere = path + "/levels/" + folderName;
            string[] split;
            int errors = 0;

            string[] paths = Directory.GetFiles(pathHere);
            StrawbMax = 0;


            foreach (string s in paths) {
                split = s.Split('\\');
                string name = split[split.Length - 1];

                if (!name.Contains(".txt"))
                    continue;

                name = name.Replace(".txt", "");

                List<byte> data = new List<byte>();

#if !TempFile
                try
                {
#endif
                output_C.Add("");

                string[] fileCont = File.ReadAllLines(s);

                if (name.Contains("_flip")) {
                    FlipRoom(s, fileCont);
                    name = name.Replace("_flip", "");
                }

                int lineIndex = 0;

                // split the first row into width/height.

                if (File.Exists(pathHere + "/" + name + ".bmp")) {
                    Bitmap map = new Bitmap(pathHere + "/" + name + ".bmp");
                    width = (byte)(map.Width);
                    height = (byte)(map.Height);
                }
                else {
                    split = fileCont[0].Split(';');
                    width = byte.Parse(split[0]);
                    height = byte.Parse(split[1]);
                    ++lineIndex;
                }


                // get the shift value based on width
                if (width > 128)
                    data.Add(0x08);
                else if (width > 64)
                    data.Add(0x07);
                else if (width > 32)
                    data.Add(0x06);
                else
                    data.Add(0x05);

                // Add width/height to data
                data.Add((byte)width);
                data.Add((byte)height);

                //Save Points
                if (File.Exists(pathHere + "/" + name + ".bmp")) {
                    Bitmap map = new Bitmap(pathHere + "/" + name + ".bmp");
                    List<Color> palettes = new List<Color>(map.Palette.Entries);

                    byte prevX = 0;
                    byte prevY = 0;
                    int amount = 0;

                    for (byte y = 0; y < height; ++y) {
                        for (byte x = 0; x < width; ++x) {
                            if (palettes.IndexOf(map.GetPixel(x, y)) == V_PLAYER) {
                                ++amount;
                                data.Add(x);
                                data.Add((byte)(y + 1));
                                prevX = x;
                                prevY = (byte)(y + 1);
                            }
                        }
                    }
                    while (amount < 4) {
                        ++amount;
                        data.Add(prevX);
                        data.Add(prevY);
                    }
                }
                else {
                    // split for save point 1 (and then 2, 3, and 4)
                    split = fileCont[lineIndex].Split(';');
                    data.Add(byte.Parse(split[0]));
                    data.Add(byte.Parse(split[1]));
                    // 2
                    split = fileCont[lineIndex + 1].Split(';');
                    data.Add(byte.Parse(split[0]));
                    data.Add(byte.Parse(split[1]));
                    // 3
                    split = fileCont[lineIndex + 2].Split(';');
                    data.Add(byte.Parse(split[0]));
                    data.Add(byte.Parse(split[1]));
                    // 4
                    split = fileCont[lineIndex + 3].Split(';');
                    data.Add(byte.Parse(split[0]));
                    data.Add(byte.Parse(split[1]));

                    lineIndex += 4;
                }

                //next room data
                split = fileCont[lineIndex].Replace(" ", "").Split(';');
                for (int i = 0; i < 4; ++i) {
                    if (split[i] == "end")
                        data.Add(254);
                    else if (!rooms.Contains(split[i]))
                        data.Add(255);
                    else
                        data.Add((byte)(rooms.IndexOf(split[i])));
                }
                ++lineIndex;


                if (File.Exists(pathHere + "/" + name + ".bmp")) {
                    //CompileLayer(pathHere + "/" + name + ".bmp", data, true);

                    if (File.Exists(pathHere + "/" + name + "_bg.bmp")) {
                        //CompileLayer(pathHere + "/" + name + "_bg.bmp", data, false);
                    }
                    else {
                        for (int size = 0; size < width * height; size += 255) {
                            data.Add(255);
                            data.Add(0x00);
                            data.Add(0x00);
                        }
                    }
                }
                else {
                    int start = lineIndex;
                    // add collision data
                    CompileLayer(ref lineIndex, fileCont, start + height, data);
                }

                for (; lineIndex < fileCont.Length; ++lineIndex) {
                    split = fileCont[lineIndex].Split(';');

                    byte type = byte.Parse(split[0]);

                    data.Add(type);

                    data.Add(byte.Parse(split[1]));
                    data.Add(byte.Parse(split[2]));

                    if (type == 1) {
                        data.Add(byte.Parse(split[3]));
                        data.Add((byte)StrawbCount);
                        ++StrawbCount;
                        if (int.Parse(split[3]) < 2)
                            ++StrawbMax;
                    }
                    if (type == 5) {
                        data.Add(byte.Parse(split[3]));
                        data.Add(byte.Parse(split[4]));
                    }
                    if (type == 11) {
                        data.Add(byte.Parse(split[3]));
                    }
                }

                data.Add(0);
#if !TempFile
                }
                catch (IndexOutOfRangeException e)
                {
                    Console.WriteLine("File " + folderName + ".txt is in an improper format.");
                    errors |= 1;
                    continue;
                }

#endif

                //*
                WriteByteArray(output_C, output_H, data.ToArray(), folderName + "_" + name);
                /*/
                output_H.Add("extern const unsigned char " + folderName + "_" + name + "[" + data.Count + "];");
                output_H.Add("");

                output_C.Add("const unsigned char " + folderName + "_" + name + "[" + data.Count + "]=");
                output_C.Add("{");

                string line = "\t";
                for (int i = 0; i < data.Count; ++i) {
                    line += "0x" + data[i].ToString("X2") + ",";

                }
                output_C.Add(line);
                int len = line.Length;
                output_C.Add("};");
                //*/

                LevelDataSize += data.Count;
            }
            output_H.Add("#define " + folderName + "_strawbMax " + StrawbMax);

            return errors;
        }
        static int CompileSprites() {
            if (!Directory.Exists(path + "/sprites"))
                return 0;

            List<string> output_C = new List<string>();
            List<string> output_H = new List<string>();

            output_H.Add("#ifndef __SPRITES__\n#define __SPRITES__\n");
            output_C.Add("#include \"sprites.h\"\n");

            string[] getFiles = Directory.GetFiles(path + "/sprites");

            foreach (string s in getFiles) {
                if (!s.Contains(".bmp")) {
                    if (!s.Contains(".ase") && !s.Contains("aseprite"))
                        Console.WriteLine("Only .bmp images are supported.");
                    continue;
                }
                string[] split = s.Split('\\');
                string name = split[split.Length - 1];
                name = name.Substring(0, name.Length - 4);

                Bitmap map = new Bitmap(s);

                int length = map.Palette.Entries.Length;
                List<Color> palette = new List<Color>(map.Palette.Entries);
                List<ushort> data = new List<ushort>();

                if (s.Contains("_pal")) {

                    for (int i = 0; i < 16; ++i) {
                        ushort color = 0;
                        Color raw = map.GetPixel(i, 0);
                        byte r = (byte)((raw.R & 0xF8) >> 3);
                        byte g = (byte)((raw.G & 0xF8) >> 3);
                        byte b = (byte)((raw.B & 0xF8) >> 3);
                        color = (ushort)(r | (g << 5) | (b << 10));

                        data.Add(color);
                    }

                    WriteUIntArray(output_C, output_H, data, name);
                }
                else {

                    for (int yL = 0; yL < map.Height >> 3; ++yL) {
                        for (int xL = 0; xL < map.Width >> 3; ++xL) {
                            for (int y = 0; y < 8; ++y) {
                                ushort tempValue = 0;
                                for (int i = 3; i >= 0; --i)
                                    tempValue = (ushort)((tempValue << 4) | palette.IndexOf(map.GetPixel((xL << 3) + i, (yL << 3) + y)));
                                data.Add(tempValue);
                                for (int i = 7; i >= 4; --i)
                                    tempValue = (ushort)((tempValue << 4) | palette.IndexOf(map.GetPixel((xL << 3) + i, (yL << 3) + y)));
                                data.Add(tempValue);
                            }
                        }
                    }

                    WriteUIntArray(output_C, output_H, data, name);
                }
                ArtDataSize += data.Count;
            }

            output_H.Add("#endif");

            File.WriteAllLines(path + "/source/sprites.c", output_C.ToArray());
            File.WriteAllLines(path + "/source/sprites.h", output_H.ToArray());

            return 0;
        }
        static int CompileBackgrounds() {

            string[] directories = Directory.GetDirectories(path + "/sprites");

            List<string> _c = new List<string>(), _h = new List<string>();

            int retVal = 0;

            foreach (string s in directories) {
                retVal |= CompileBackground(s, _c, _h);
            }

            File.WriteAllLines(path + "/source/backgrounds.c", _c.ToArray());
            File.WriteAllLines(path + "/source/backgrounds.h", _h.ToArray());

            return retVal;
        }
        static int CompileBackground(string path, List<string> _c, List<string> _h) {
            List<ushort> data;
            ushort tempValue = 0;

            string[] split = path.Split('\\');
            string name = split[split.Length - 1];

            byte[] fileData = File.ReadAllBytes(path + "/BG_Maps.raw");
            int w = 64, h = 64;
            int shift = 7, bgOffset;
            bool hasBG = fileData.Length > 8192;
            byte paletteOffset = 0;
            ushort tileOffset = 0;

            if (File.Exists(path + "/exData.txt"))
            {
                string[] extraData = File.ReadAllLines(path + "/exData.txt");

                hasBG = extraData[0] != "0";
                w = int.Parse(extraData[1]);
                if (w == 32)
                    shift = 6;
                h = int.Parse(extraData[2]);
                if (extraData.Length > 3)
                    paletteOffset = byte.Parse(extraData[3]);
                if (extraData.Length > 4)
                    tileOffset = ushort.Parse(extraData[4]);
            }

            bgOffset = w * h * 2;

            ushort[] foregroundData = new ushort[w * h], backgroundData = new ushort[w * h];

            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x) {
                    if (hasBG) {
                        tempValue = (ushort)(fileData[(x << 1) + (y << shift) + bgOffset] + (fileData[(x << 1) + (y << shift) + 1 + bgOffset] << 8));
                        tempValue = (ushort)((tempValue & 0x0FFF) | ((tempValue & 0xF000) + (paletteOffset << 12)));

                        tempValue += tileOffset;

                        backgroundData[(x & 0x1F) + ((y & 0x1F) << 5) + ((x & 0x20) << 5) + ((y & 0x20) << 6)] = tempValue;
                    }

                    tempValue = (ushort)(fileData[(x << 1) + (y << shift)] + (fileData[(x << 1) + (y << shift) + 1] << 8));
                    tempValue = (ushort)((tempValue & 0x0FFF) | ((tempValue & 0xF000) + (paletteOffset << 12)));

                    tempValue += tileOffset;

                    foregroundData[(x & 0x1F) + ((y & 0x1F) << 5) + ((x & 0x20) << 5) + ((y & 0x20) << 6)] = tempValue;
                }
            }
            data = new List<ushort>(foregroundData);
            WriteUshortArray(_c, _h, data, name + "_fg");
            if (hasBG) {
                data = new List<ushort>(backgroundData);
                WriteUshortArray(_c, _h, data, name + "_bg");
            }

            fileData = File.ReadAllBytes(path + "/BG_Tiles.raw");
            data = new List<ushort>();

            for (int i = 0; i < fileData.Length; i += 2) {
                tempValue = (ushort)(fileData[i] + (fileData[i + 1] << 8));
                data.Add(tempValue);
            }

            WriteUshortArray(_c, _h, data, name + "_tileset");

            return 0;
        }

        static int GetPitch(int _octave, string _pitch) {
            return 2048 - (notes[_pitch] >> (2 + _octave));
        }
        static int GetPitch(int _octave, int _pitch) {

            string[] indexes = new string[] { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

            return 2048 - (notes[indexes[_pitch]] >> (_octave));
        }

        static void AddNoteSQ1(List<ushort> _data, int _pitch, int _volume, int _length, int _sweep, int _sweepLength, int _crescendo, int _cycle) {
            ushort value = (ushort)((_sweep > 0 ? 0 : 1) << 3);
            value |= (ushort)(Math.Abs(_sweep));
            value |= (ushort)(_sweepLength << 4);

            AddNoteSQ2(_data, _pitch, _volume, value, _crescendo, _cycle);

            _data.Add((ushort)_length);
        }
        static void AddNoteSQ2(List<ushort> _data, int _pitch, int _volume, int _length, int _crescendo, int _cycle) {
            ushort value = (ushort)(_volume << 12);
            value |= (ushort)((_crescendo > 0 ? 0 : 1) << 11);
            value |= (ushort)((8 - Math.Abs(_crescendo)) << 8);
            value |= (ushort)((_cycle & 0x3) << 6);

            _data.Add(value);
            _data.Add((ushort)_pitch);
            _data.Add((ushort)_length);
        }
        static void AddNoteSQ3(List<ushort> _data, int _pitch, int _volume, int _length) {
            if (_volume > 3) // 100
                _volume = 0x1;
            else if (_volume == 3) // 75
                _volume = 0x4;
            else if (_volume == 2) // 50
                _volume = 0x2;
            else if (_volume == 1) // 25
                _volume = 0x3;
            else
                _volume = 0;

            ushort value = (ushort)(_volume << 13);

            _data.Add(value);
            _data.Add((ushort)_pitch);
            _data.Add((ushort)_length);
        }
        static void WriteUshortArray(List<string> _c, List<string> _h, List<ushort> data, string name) {
            _h.Add("#define " + name + "Len " + (data.Count << 1));
            _h.Add("extern const unsigned short " + name + "[" + data.Count + "];");
            _h.Add("");

            _c.Add("const unsigned short " + name + "[" + data.Count + "]=");
            _c.Add("{");

            for (int i = 0; i < data.Count; i += ARRAY_WIDTH_USHORT) {
                string line = "\t";
                for (int j = 0; j < ARRAY_WIDTH_USHORT && (i + j < data.Count); ++j)
                    line += "0x" + data[i + j].ToString("X4") + ",";

                _c.Add(line);
            }
            _c.Add("};");
        }
        static void WriteUIntArray(List<string> _c, List<string> _h, List<ushort> data, string name) {
            _h.Add("#define " + name + "Len " + (data.Count << 1));
            _h.Add("extern const unsigned int " + name + "[" + (data.Count >> 1) + "];");
            _h.Add("");

            _c.Add("const unsigned int " + name + "[" + (data.Count >> 1) + "]=");
            _c.Add("{");

            for (int i = 0; i < data.Count; i += ARRAY_WIDTH_USHORT) {
                string line = "\t";
                for (int j = 0; j < ARRAY_WIDTH_USHORT && (i + j < data.Count); j += 2)
                    line += "0x" + data[i + j + 1].ToString("X4") + data[i + j].ToString("X4") + ",";

                _c.Add(line);
            }
            _c.Add("};");
        }
        static void WriteByteArray(List<string> _c, List<string> _h, byte[] data, string name) {
            _h.Add("#define " + name + "Len " + data.Length);
            _h.Add("extern const char " + name + "[" + data.Length + "];");
            _h.Add("");

            _c.Add("const char " + name + "[" + data.Length + "]= {");

            StringBuilder builder = new StringBuilder(data.Length * 4);

            for (int i = 0; i < data.Length; ++i) {
                builder.Append("0x" + data[i].ToString("X2") + ",");

            }
            string s = builder.ToString();
            _c.Add(builder.ToString());

            _c.Add("};");
        }

        static byte Char2TextData(char _c, char _previous) {
            byte value = (byte)(char.ToLower(_c));

            if ((_c == ' ' && _previous == '.') || _c == '$')
                return 0xF0;
            if (_c == ' ')
                return 36;
            if (_c == '.')
                return 37;
            if (_c == '!')
                return 38;
            if (_c == '?')
                return 39;
            if (_c == '\'')
                return 40;
            if (_c == ',')
                return 42;


            if (value >= 48 && value <= 57)
                value -= 48;
            else if (value >= 97 && value <= 122)
                value -= 87;


            return value;
        }
        static bool TileTransparent(int _x, int _y) {
            byte tile = ground.GetByte(_x, _y, true);

            if (tile == V_PLATFORM || tile == 0 || tile == V_WATER)
                return true;
            if (tile == V_SOLID_1 || tile == V_SOLID_2 || tile == V_SOLID_3)
                return false;

            int offset = 0;

            if (VisualsMatch(tile, ground.GetByte(_x + 1, _y, true))) // Right   - 1
                offset |= 1;
            if (VisualsMatch(tile, ground.GetByte(_x - 1, _y, true))) // Left    - 2
                offset |= 2;
            if (VisualsMatch(tile, ground.GetByte(_x, _y - 1, true))) // Up      - 4
                offset |= 4;
            if (VisualsMatch(tile, ground.GetByte(_x, _y + 1, true))) // Down    - 8
                offset |= 8;

            if (offset == 15)
                return false;

            return true;
        }

        /// <summary>
        /// Compile from textfile
        /// </summary>
        /// <param name="lineIndex"></param>
        /// <param name="fileCont"></param>
        /// <param name="max"></param>
        /// <param name="data"></param>
        /// <param name="collidable"></param>
        static void CompileLayer(ref int lineIndex, string[] fileCont, int max, List<byte> data) {
            bool collidable = true;
            byte count = 0;

            ground.SetData(lineIndex, width, height, fileCont);

            for (int i = 0; i < 2; ++i) {
                bool useColl = collidable || DoubleCollision;

                char value = fileCont[lineIndex][0];

                for (; lineIndex < max; ++lineIndex) {
                    string fileStr = fileCont[lineIndex];
                    for (int ch = 0; ch < fileStr.Length; ++ch) {
                        ground.SetByte(ch, lineIndex - (max - height), collidable, Char2ByteVis(fileStr[ch]));

                        if (!useColl)
                            continue;

                        if (fileStr[ch] != value || count == 255) {
                            data.Add(count);
                            data.Add(Char2CollByte(value));
                            count = 0;
                            value = fileStr[ch];
                        }
                        ++count;
                    }
                }
                // finalize collision data
                if (useColl) {
                    data.Add((byte)(Math.Min(count + 3, 255)));
                    data.Add(Char2CollByte(value));
                }

                if (i == 0) {
                    if (lineIndex < fileCont.Length && fileCont[lineIndex].ToLower().StartsWith("bg")) {
                        collidable = false;
                        ++lineIndex;
                        max += height + 1;
                    }
                    else {
                        ground.BG = new byte[width, height];

                        for (int x = 0; x < width; ++x)
                            for (int y = 0; y < height; ++y)
                                ground.BG[x, y] = 0;

                        break;
                    }
                }
            }

            collidable = true;
            for (int i = 0; i < 2; ++i) {
                count = 0;
                ushort block = GetVisualBlock(0, 0, collidable);
                ushort visValue = GetVisualBlock(0, 0, collidable);

                for (int y = 0; y < height; ++y) {
                    for (int x = 0; x < width; ++x) {
                        if (collidable || (x == 0 && y == 0) || count == 255 || TileTransparent(x, y))
                            block = GetVisualBlock(x, y, collidable);
                        else
                            block = visValue;

                        if (visValue != block || count == 255) {
                            data.Add(count);
                            data.Add((byte)(visValue & 0xFF));
                            data.Add((byte)(0xFF & visValue >> 8));
                            count = 0;
                            visValue = GetVisualBlock(x, y, collidable);
                        }
                        ++count;
                    }
                }
                data.Add((byte)(Math.Min(count + 3, 255)));
                data.Add((byte)(visValue & 0xFF));
                data.Add((byte)(0xFF & visValue >> 8));

                collidable = false;
            }
        }
        static byte Char2CollByte(char value) {
#if CELESTE
            switch (value) {
                case 'M':
                case 'N':
                case 'O':
                case 'r':
                    return C_SOLID;
                case 'Q':
                    return C_NOTE1;
                case 'E':
                    return C_NOTE2;
                case 'w':
                    return C_WATER;
                case 'S':
                    return C_STRAWB;
                case '-':
                    return C_PLATFORM;
                case 'D':
                    return C_DREAM;
                case 'A':
                    return C_SPIKE_U;
                case 'V':
                    return C_SPIKE_D;
                case '.':
                    return C_SPIKE_L;
                case ',':
                    return C_SPIKE_R;
                case 'X':
                case 'x':
                    return C_SPINNER;
                default:
                    return 0;
            }
#else
            switch (value)
            {
            case 'M':
            case 'N':
                return C_SOLID;
            case '-':
                return C_PLATFORM;
            case 'A':
                return 4;
            case 'V':
                return 5;
            case '.':
                return 6;
            case ',':
                return 7;
            default:
                return 0;
            }
#endif
        }
        static byte VisByte2CollByte(byte value) {
#if CELESTE
            switch (value) {
                case V_SOLID_1:
                case V_SOLID_2:
                case V_SOLID_3:
                    return C_SOLID;
                case V_SPINNER:
                    return C_SPINNER;
                case V_PLATFORM:
                    return C_PLATFORM;
                default:
                    return 0;
            }
#endif
        }
        static byte Char2ByteVis(char value) {
            switch (value) {
                case 'M':
                case 'm':
                    return V_SOLID_1;
                case 'N':
                case 'n':
                    return V_SOLID_2;
                case 'O':
                case 'o':
                    return V_SOLID_3;
                case 'Q':
                    return V_NOTE1;
                case 'E':
                    return V_NOTE2;
                case 'w':
                    return V_WATER;
                case 'R':
                    return V_SOLIDBG_1;
                case 'T':
                    return V_SOLIDBG_2;
                case 'Y':
                    return V_SOLIDBG_3;
                case '-':
                    return V_PLATFORM;
                case 'D':
                    return V_DREAMBLOCK;
                case 'A':
                    return V_SPIKEUP;
                case 'V':
                    return V_SPIKEDOWN;
                case '.':
                    return V_SPIKERIGHT;
                case ',':
                    return V_SPIKELEFT;
                case 'X':
                case 'x':
                    return V_SPINNER;
                default:
                    return 0;
            }
        }
        static ushort GetDecoValue(string[] _split) {
            ushort retval = ushort.Parse(_split[1]);

            for (int i = 2; i < _split.Length; ++i) {
                var split = _split[i].Split('-');
                switch (split[0].ToLower()) {
                    case "v":
                        retval |= 0x800;
                        break;
                    case "h":
                        retval |= 0x400;
                        break;
                }
            }

            return retval;
        }
        static ushort GetVisualBlock(int x, int y, bool foreground) {
            // Set the palette value
            byte value = ground.GetByte(x, y, foreground);

            bool fg = foreground;

            if (value == V_SOLIDBG_1 || value == V_SOLIDBG_2 || value == V_SOLIDBG_3) {
                foreground = false;
            }

            if (value == 0) {

                char c = ground.GetData(x, y, foreground);
                if (decoration.ContainsKey(c))
                    return (ushort)(decoration[c] + (!foreground ? 0x3000 : 0));

                if (!foreground)
                    return 0;

                if (ground.GetByte(x, y - 1, true) == V_SPIKERIGHT)
                    return (ushort)((10 << 12) + 7);
                if (ground.GetByte(x, y + 1, true) == V_SPIKELEFT)
                    return (ushort)((10 << 12) + 8);
                if (ground.GetByte(x - 1, y, true) == V_SPIKEUP)
                    return (ushort)((10 << 12) + 5);
                if (ground.GetByte(x + 1, y, true) == V_SPIKEDOWN)
                    return (ushort)((10 << 12) + 6);

                return 0;
            }

            byte block;
            ushort retval = 0;

            int blockIndex = 0;
            int flipIndex = 0;

            int offset = 0;

            // Get index of blocks surrounding block
            if (VisualsMatch(value, block = ground.GetByte(x + 1, y, fg))) // Right   - 1
                offset |= 1;

            if (VisualsMatch(value, block = ground.GetByte(x - 1, y, fg))) // Left    - 2
                offset |= 2;

            if (VisualsMatch(value, block = ground.GetByte(x, y - 1, fg))) // Up      - 4
                offset |= 4;

            if (VisualsMatch(value, block = ground.GetByte(x, y + 1, fg))) // Down    - 8
                offset |= 8;

            if (value == V_PLATFORM) {
                retval = (ushort)(10 << 12);

                int left = ground.GetByte(x - 1, y, fg),
                    right = ground.GetByte(x + 1, y, fg);

                blockIndex = 9;

                if (left <= 4 && left >= 1)
                    blockIndex += 0;
                else if (right <= 4 && right >= 1)
                    blockIndex += 1;
                else if (left == 0)
                    blockIndex += 2;
                else if (right == 0)
                    blockIndex += 3;
                else
                    blockIndex += 4 + randomizer.Next(0, 3);


            }
            else if (value >= 4 && value <= 7) {
                retval = (ushort)(10 << 12);

                byte against;
                if (value == V_SPIKEUP)
                    against = ground.GetByte(x, y + 1, fg);
                else if (value == V_SPIKEDOWN)
                    against = ground.GetByte(x, y - 1, fg);
                else if (value == V_SPIKELEFT)
                    against = ground.GetByte(x + 1, y, fg);
                else
                    against = ground.GetByte(x - 1, y, fg);

                if (against == V_NOTE1)
                    retval = (ushort)(13 << 12);
                else if (against == V_NOTE2)
                    retval = (ushort)(14 << 12);

                blockIndex = value - 3;
            }
            else if (value == V_WATER) {
                blockIndex = 128;

                switch (offset) {
                    // Corners
                    case 9:
                        blockIndex += 0;
                        break;
                    case 10:
                        blockIndex += 1;
                        break;
                    case 5:
                        blockIndex += 2;
                        break;
                    case 6:
                        blockIndex += 3;
                        break;
                    // Edges
                    case 11:
                        blockIndex += 4 + randomizer.Next(0, 2);
                        break;
                    case 7:
                        blockIndex += 6 + randomizer.Next(0, 2);
                        break;
                    case 13:
                        blockIndex += 8 + randomizer.Next(0, 2);
                        break;
                    case 14:
                        blockIndex += 10 + randomizer.Next(0, 2);
                        break;
                    //Center
                    default:
                        blockIndex += 12 + randomizer.Next(0, 4);
                        break;
                }
                retval = (ushort)(11 << 12);
            }
            else if (value == V_DREAMBLOCK || value == V_NOTE1 || value == V_NOTE2) {
                retval = (ushort)(11 << 12);

                blockIndex = value == V_DREAMBLOCK ? 128 : 16;

                switch (offset) {
                    case 5:
                        blockIndex += 2;
                        break;
                    case 6:
                        blockIndex += 3;
                        break;
                    case 9:
                        blockIndex += 0;
                        break;
                    case 10:
                        blockIndex += 1;
                        break;
                    case 7:
                        blockIndex += 7;
                        break;
                    case 11:
                        blockIndex += 6;
                        break;
                    case 13:
                        blockIndex += 4;
                        break;
                    case 14:
                        blockIndex += 5;
                        break;
                    default:
                        blockIndex += 9;
                        break;
                }

                if (value != V_DREAMBLOCK) {
                    retval = (ushort)((value == V_NOTE1 ? 13 : 14) << 12);
                }
            }
            else if (value == V_SPINNER) {
                retval = (ushort)(15 << 12);

                switch (offset) {
                    case 0: //Solo blocks are illegal (for now)
                        blockIndex = 0 + randomizer.Next(0, 3);
                        flipIndex = randomizer.Next(0, 4);
                        break;
                    case 9: // top left
                    case 10: // top right
                    case 5: // bottomleft
                    case 6: // bottomright
                        blockIndex = 0 + randomizer.Next(0, 3);
                        flipIndex = ((offset & 0x1) == 0 ? 1 : 0) |
                                    ((offset & 0x4) == 0 ? 0 : 2);
                        break;
                    case 7: // top edge
                    case 11: // bottom edge
                        blockIndex = 3 + randomizer.Next(0, 3);
                        flipIndex = (offset == 11 ? 0 : 2);
                        break;
                    case 13: // left edge
                    case 14: // right edge
                        blockIndex = 6 + randomizer.Next(0, 3);
                        flipIndex = (offset == 13 ? 0 : 1);
                        break;
                    case 3:
                    case 12:
                    case 1:
                    case 2:
                    case 4:
                    case 8:
                        throw new Exception();
                    default:

                        offset = 0;

                        if (!VisualsMatch(value, x - 1, y - 1, fg))
                            offset = 1;
                        if (!VisualsMatch(value, x + 1, y - 1, fg))
                            offset = 2;
                        if (!VisualsMatch(value, x - 1, y + 1, fg))
                            offset = 4;
                        if (!VisualsMatch(value, x + 1, y + 1, fg))
                            offset = 8;

                        if (offset == 0 && (VisualsMatch(value, x - 2, y, fg) && VisualsMatch(value, x, y - 2, fg) && VisualsMatch(value, x + 2, y, fg) && VisualsMatch(value, x, y + 2, fg))) {
                            blockIndex = 14 + randomizer.Next(0, 2);
                            flipIndex = randomizer.Next(0, 4);
                        }
                        else if (offset != 0) {
                            blockIndex = 9 + randomizer.Next(0, 2);
                            flipIndex = ((offset & 0xA) != 0 ? 1 : 0) |
                                        ((offset & 0xC) != 0 ? 2 : 0);
                        }
                        else {
                            blockIndex = 11 + randomizer.Next(0, 3);
                            flipIndex = randomizer.Next(0, 4);
                        }

                        break;
                }
                blockIndex += 128;
            }
            else {

                retval = (ushort)((value + 3 + (foreground ? 0 : 3)) << 12);

                switch (offset) {
                    case 0: //Solo blocks are illegal (for now)
                        throw new Exception();
                    case 9: // top left
                    case 10: // top right
                        blockIndex = 0 + randomizer.Next(0, 3);
                        flipIndex = offset == 10 ? 1 : 0;
                        break;
                    case 5: // bottomleft
                    case 6: // bottomright
                        blockIndex = 3 + randomizer.Next(0, 3);
                        flipIndex = offset == 6 ? 1 : 0;
                        break;
                    case 7: // top edge
                        blockIndex = 12 + randomizer.Next(0, 3);
                        break;
                    case 11: // bottom edge
                        blockIndex = 6 + randomizer.Next(0, 3);
                        break;
                    case 13: // left edge
                    case 14: // right edge
                        blockIndex = 9 + randomizer.Next(0, 3);
                        flipIndex = (offset == 13 ? 0 : 1);
                        break;
                    case 3:
                        blockIndex = 20 + randomizer.Next(0, 2);
                        break;
                    case 12:
                        blockIndex = 18 + randomizer.Next(0, 2);
                        break;
                    case 1:
                    case 2:
                        blockIndex = 16;
                        flipIndex = (offset == 1 ? 0 : 1);
                        break;
                    case 4:
                        blockIndex = 17;
                        flipIndex = randomizer.Next(0, 2);
                        break;
                    case 8:
                        blockIndex = 15;
                        flipIndex = randomizer.Next(0, 2);
                        break;
                    default:
                        blockIndex = 22 + randomizer.Next(0, 8);
                        if (foreground) {
                            if (VisualsMatch(value, x - 2, y, fg) && VisualsMatch(value, x, y - 2, fg) && VisualsMatch(value, x + 2, y, fg) && VisualsMatch(value, x, y + 2, fg) &&
                                VisualsMatch(value, x - 1, y - 1, fg) && VisualsMatch(value, x + 1, y - 1, fg) && VisualsMatch(value, x - 1, y + 1, fg) && VisualsMatch(value, x + 1, y + 1, fg))
                                blockIndex = 30 + randomizer.Next(0, 2);
                            flipIndex = randomizer.Next(0, 4);
                        }
                        break;
                }
                if (value >= 17)
                    value -= 16;

                blockIndex += 32 * value;
                
            }


            retval |= (ushort)blockIndex;
            retval |= (ushort)(flipIndex << 10);

            return retval;
        }
        static bool VisualsMatch(byte center, byte extra) {
            if (center == extra)
                return true;

            switch (center) {
                case V_SOLIDBG_1:
                case V_SOLID_1:
                    if (extra == V_SOLID_1 || extra == V_SOLID_2 || extra == V_SOLID_3)
                        return true;
                    return false;
                case V_SOLIDBG_2:
                case V_SOLID_2:
                    if (extra == V_SOLID_2)
                        return true;
                    return false;
                case V_SOLIDBG_3:
                case V_SOLID_3:
                    if (extra == V_SOLID_1 || extra == V_SOLID_2 || extra == V_SOLID_3)
                        return true;
                    return false;
                case V_SPINNER:
                    if (extra == V_SOLID_1 || extra == V_SOLID_2 || extra == V_SOLID_3)
                        return true;
                    return false;
                case V_PLATFORM:
                    if (extra != 0 && extra != V_PLATFORM)
                        return true;

                    return false;
                default:
                    return center == extra;
            }
        }
        static bool VisualsMatch(byte center, int x, int y, bool fg) {
            if (x < 0)
                x = 0;
            if (y < 0)
                y = 0;
            if (x > width - 1)
                x = width - 1;
            if (y > height - 1)
                y = height - 1;

            return VisualsMatch(center, ground.GetByte(x, y, fg));
        }
    }
}
