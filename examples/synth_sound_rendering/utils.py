import numpy as np
import matplotlib.pyplot as plt
import librosa as lb
import librosa.display as lbd
import os
import random
import json
import xmltodict
import difflib
import re
import math

def piano_note_to_midi_note(piano_note):
    """
    Convert a string representation of a piano note to its corresponding MIDI note number.
    
    Args:
        piano_note (str): A string representation of a piano note (e.g. 'C4').
    
    Returns:
        int: The MIDI note number corresponding to the input piano note.
    """
    # Define lists of piano note names and their corresponding MIDI note numbers
    piano_notes = ['A', 'A#', 'B', 'C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#']
    midi_notes = [21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32]

    # Extract the octave and note name from the piano note input
    octave = int(piano_note[-1])
    note_name = piano_note[:-1]

    # Find the index of the note name in the piano_notes list and add the corresponding MIDI note number
    note_num = piano_notes.index(note_name)
    midi_note = midi_notes[note_num] + (octave + 1) * 12

    return midi_note

def read_txt(path: str) -> str:
    """
    Read the contents of a text file and return as a string.
    
    Args:
        path (str): The path to the text file to be read.
    
    Returns:
        str: The contents of the text file as a string.
    """
    with open(path, 'r') as file:
        txt = file.read()
    return txt

def get_xml_preset_settings(preset_path: str):
    """
    Read a preset file in XML format and convert it to a dictionary.
    
    Args:
        preset_path (str): The path to the preset file.
    
    Returns:
        str: The preset settings in JSON format as a string.
    """
    # read the preset_path using with ... as 'rb' ... etc.
    txt = read_txt(preset_path)
    preset_settings = None

    # Assuming the presence of "<?xml" at the start of the file indicates an XML file
    if txt.strip().startswith("<?xml"):
        try:
            # Convert XML to a dictionary
            xml_dict = xmltodict.parse(txt)

            # Convert the dictionary to JSON (optional)
            preset_settings = json.dumps(xml_dict)
        except Exception as e:
            print(f"Error: {e}")
            raise ValueError("Unable to parse XML file")
    else:
        raise ValueError("Unsupported file type")

    return preset_settings

def make_json_parameter_mapping(plugin, preset_path:str, verbose=True):
    """
    Read a preset file in XML format, apply the settings to the plugin, and create a JSON file
    that maps the preset parameters to the plugin parameters.
    
    Args:
        plugin (dawdreamer.PluginProcessor): The plugin to which the preset settings will be applied.
        preset_path (str): The path to the preset file in XML format.
        verbose (bool): if True, it will print parameter mapping. Default is True.
    
    Returns:
        str: The name of the JSON file containing the parameter mapping.
    """
    # create the json preset folder if it does not already exist
    json_preset_folder = f'TAL-UNO_json_presets'
    if not os.path.exists(json_preset_folder):
        os.mkdir(json_preset_folder)

    # specify the output json filename
    preset_name = preset_path.split(os.sep)[-1].split('.pjunoxl')[0]
    output_name = f'{json_preset_folder}{os.sep}TAL-UNO-{preset_name}-parameter-mapping.json'

    if not os.path.exists(output_name):
        # read the XML preset path
        preset_settings = get_xml_preset_settings(preset_path)

        # apply the synth preset settings to the synth plugin processor object
        parameter_mapping = {}

        # Load JSON settings
        settings = json.loads(preset_settings)

        # Extract the program settings
        json_keys = settings["tal"]["programs"]["program"]

        # Get the parameters description from the plugin
        parameters = plugin.get_parameters_description()

        # Create a dictionary with parameter names as keys and their indices as values
        param_name_to_index = {param["name"]: param["index"] for param in parameters}

        # Iterate over each JSON key
        for key in json_keys:
            # specify the exceptions to map manually
            exceptions = {
                'volume':'master volume', 
                'octavetranspose':'master octave transpose',
                'adsrdecay':'decay',
                'adsrsustain':'sustain',
                'adsrrelease':'release',
                'chorus1enable':'chorus 1',
                'chorus2enable':'chorus 2',
                'midiclocksync':'clock sync',
                'miditriggerarp16sync':'trigger arp by midi channel 16'
                }

            if key.split('@')[-1] not in exceptions: # find the closest match automatically           
                # Find the closest match in the plugin parameter name list using max() and difflib.SequenceMatcher
                closest_match = max(param_name_to_index.keys(), key=lambda param_name: difflib.SequenceMatcher(None, key, param_name).ratio())

                if key.split('@')[-1][0] == closest_match[0]: # only continue if the first letters are the same and specified exceptions
                    if verbose:
                        print(f'match found for {key}; closest match: {closest_match}')
                    # Extract the value of the JSON key from the JSON string using regex
                    match_value = re.search(r'"{}":\s*"([\d.]+)"'.format(key), preset_settings)
                    if match_value:
                        param_value = float(match_value.group(1))
                        index = param_name_to_index[closest_match]
                        parameter_mapping[key] = {'match': closest_match, 'value': param_value, 'index': index}
                else:
                    if verbose:
                        print(f'no match found for {key}; closest match: {closest_match}')
            else:
                # map manually
                key_temp = key.split('@')[-1]

                # get closest_match from exceptions list
                closest_match = exceptions[key_temp]

                # Extract the value of the JSON key from the JSON string using regex
                match_value = re.search(r'"{}":\s*"([\d.]+)"'.format(key), preset_settings)
                if match_value:
                    param_value = float(match_value.group(1))
                    index = param_name_to_index[closest_match]

                parameter_mapping[key] = {'match': closest_match, 'value': param_value, 'index': index}
        
        
        with open(output_name, 'w') as outfile:
            json.dump(parameter_mapping, outfile)  

    return output_name      

def load_xml_preset(dawdreamer_plugin,parameter_mapping_json):
    """
    Load a preset into a plugin using a JSON file that maps preset parameters to plugin parameters.
    
    Args:
        dawdreamer_plugin (dawdreamer.PluginProcessor): The plugin to which the preset settings will be applied.
        parameter_mapping_json (str): The path to the JSON file that maps preset parameters to plugin parameters.
    Returns:
        dawdreamer.PluginProcessor: The plugin with the preset settings applied.
    """
    # Load JSON file into a dictionary
    with open(parameter_mapping_json, 'r') as infile:
        parameter_map = json.load(infile)

    # Get the parameters description from the plugin
    parameters = dawdreamer_plugin.get_parameters_description()

    # Create a dictionary with parameter names as keys and their indices as values
    param_name_to_index = {param["name"]: param["index"] for param in parameters}

    # Iterate over each JSON key
    for key in parameter_map.keys():
        dawdreamer_plugin.set_parameter(parameter_map[key]['index'], parameter_map[key]['value'])
    
    return dawdreamer_plugin

def select_preset_path(folder_path, preset_ext):
    """
    Select a random preset file path from a preset folder path and its subdirectories.
    
    Args:
        folder_path (str): The path to the folder to search for preset files.
        preset_ext (str): The file extension of the preset files to search for (e.g. ".pjunoxl").
    
    Returns:
        str or None: The path to a randomly selected preset file, or None if no preset files are found.
    """
    preset_files = []
    for root, dirs, files in os.walk(folder_path):
        for file in files:
            if file.endswith(preset_ext):
                preset_files.append(os.path.join(root, file))
    return random.choice(preset_files) if preset_files else None

def audio2mel_spectrogram(audio_folder_path, plot_flag=False, window_size=2048, zero_padding_factor=1,
                           window_type='hann', gain_db=0.0, range_db=80.0, high_boost_db=0.0, f_min=0, f_max=20000, n_mels=256):
    """
    Convert a collection of audio files to mel-scaled spectrograms.
    
    Args:
        audio_folder_path (str): The path to the folder containing the audio files.
        plot_flag (bool, optional): Whether to plot the mel-scaled spectrograms. Defaults to False.
        window_size (int, optional): The size of the FFT window to use. Defaults to 2048.
        zero_padding_factor (int, optional): The amount of zero-padding to use in the FFT. Defaults to 1.
        window_type (str, optional): The type of window function to use in the FFT. Defaults to 'hann'.
        gain_db (float, optional): The gain to apply to the audio signal in decibels. Defaults to 0.0.
        range_db (float, optional): The range of the mel-scaled spectrogram in decibels. Defaults to 80.0.
        high_boost_db (float, optional): The amount of high-frequency boost to apply to the mel-scaled spectrogram in decibels. Defaults to 0.0.
        f_min (int, optional): The minimum frequency to include in the spectrogram (Hz). Defaults to 0.
        f_max (int, optional): The maximum frequency to include in the spectrogram (Hz). Defaults to 20000.
        n_mels (int, optional): The number of mel frequency bins to include in the spectrogram. Defaults to 256.
    
    Returns:
        list: A list of mel-scaled spectrograms, where each element is a NumPy array.
    """

    # Get a list of audio file names in the folder
    audio_file_names = os.listdir(audio_folder_path)
    np.random.shuffle(audio_file_names)
    
    # Compute mel-scaled spectrograms for each audio file
    mel_spectrograms = []
    for file_name in audio_file_names:
        audio_file_path = os.path.join(audio_folder_path, file_name)
        signal, sample_rate = lb.load(audio_file_path)

        # Apply gain to the audio signal
        signal = lb.util.normalize(signal) * lb.db_to_amplitude(gain_db)

        # Compute the mel-scaled spectrogram
        fft_size = window_size * zero_padding_factor
        hop_length = window_size // 2
        mel_filterbank = lb.filters.mel(sr=sample_rate, n_fft=fft_size, n_mels=n_mels)
        window = lb.filters.get_window(window_type, window_size, fftbins=True)
        spectrogram = np.abs(lb.stft(signal, n_fft=fft_size, hop_length=hop_length, window=window))**2
        mel_spectrogram = lb.feature.melspectrogram(S=spectrogram, sr=sample_rate, n_mels=n_mels,
                                                     fmax=f_max, htk=True, norm=None)
        mel_spectrogram = lb.power_to_db(mel_spectrogram, ref=np.max)

        # Apply range and high boost to the mel-scaled spectrogram
        mel_spectrogram = np.clip(mel_spectrogram, a_min=-range_db, a_max=None)
        mel_spectrogram = mel_spectrogram + high_boost_db

        # Plot the mel-scaled spectrogram if plot_flag is True
        if plot_flag:
            plt.figure(figsize=(10, 4))
            lbd.specshow(mel_spectrogram, x_axis='time', y_axis='mel',sr=sample_rate, fmin=f_min, fmax=f_max, hop_length=hop_length, cmap='jet', vmin=-range_db, vmax=mel_spectrogram.max() + high_boost_db)
            plt.colorbar(format='%+2.0f dB')
            plt.title('Mel spectrogram for {}'.format(file_name))
            plt.tight_layout()
            plt.show()
        
        mel_spectrograms.append(mel_spectrogram)

        if len(mel_spectrograms) > 4:
            break

    return mel_spectrograms