import re

with open('ffplayer.cpp', 'r', encoding='utf-8') as f:
    content = f.read()

# Variables to replace with is-> or s-> prefix
vars_to_fix = [
    'audio_disable', 'video_disable', 'subtitle_disable',
    'seek_by_bytes', 'seek_interval', 'startup_volume',
    'av_sync_type', 'start_time', 'duration', 'loop',
    'framedrop', 'show_mode', 'afilters', 'vfilters_list',
    'nb_vfilters', 'hwaccel', 'enable_vulkan', 'vulkan_params',
    'audio_codec_name', 'video_codec_name', 'subtitle_codec_name',
    'wanted_stream_spec'
]

# Don't replace in struct definitions, initializations, or declarations
lines = content.split('\n')
new_lines = []
in_struct = False
in_init = False

for i, line in enumerate(lines):
    new_line = line
    
    # Skip struct definition
    if 'typedef struct VideoState' in line or (in_struct and '}' not in line):
        in_struct = True
        if '}' in line and 'VideoState' in line:
            in_struct = False
        new_lines.append(new_line)
        continue
    
    # Skip initialization block in player_run
    if 'initialize per-instance playback options' in line:
        in_init = True
    if in_init and 'memset(&is->renderer_info' in line:
        in_init = False
    
    if not in_init and not in_struct:
        # Replace bare variable references with is-> or s-> prefix
        # But only if they're not already prefixed
        for var in vars_to_fix:
            # Match standalone variable references (not part of larger identifiers)
            # and not already prefixed with is-> or s->
            pattern = r'(?<!is->)(?<!s->)(?<!\w)' + var + r'(?!\w)'
            
            # Check if line has is or s variable available
            if re.search(r'\b(VideoState\s*\*\s*(is|s)|\(is\)|\(s\))', line) or \
               re.search(r'\b(is|s)->', line) or \
               'stream_component_open' in ''.join(lines[max(0,i-50):i]):
                # Replace but be careful with arrays
                if var == 'wanted_stream_spec':
                    new_line = re.sub(pattern + r'(\[)', r'is->\1', new_line)
                else:
                    new_line = re.sub(pattern, r'is->' + var, new_line)
                    # Also handle s-> case
                    if re.search(r'\bVideoState\s*\*\s*s\b', line) or '(s)' in line:
                        new_line = re.sub(r'is->' + var, r's->' + var, new_line)
    
    new_lines.append(new_line)

with open('ffplayer.cpp', 'w', encoding='utf-8') as f:
    f.write('\n'.join(new_lines))

print("Done")
