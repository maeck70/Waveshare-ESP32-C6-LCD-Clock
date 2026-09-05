import os

try:
    Import("env")
    proj_dir = env.get("PROJECT_DIR", os.getcwd())
except Exception:
    proj_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) if '__file__' in globals() else os.getcwd()

def load_env_file():
    env_file = os.path.join(proj_dir, '.env')
    out_file = os.path.join(proj_dir, 'src', 'wifi_config.h')
    
    config = {
        'WIFI_SSID': '',
        'WIFI_PASSWORD': '',
        'TIMEZONE': 'PST8PDT,M3.2.0,M11.1.0',
        'NTP_SERVER': 'pool.ntp.org'
    }
    
    if os.path.isfile(env_file):
        with open(env_file, 'r', encoding='utf-8') as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith('#'):
                    continue
                if '=' in line:
                    key, val = line.split('=', 1)
                    key = key.strip()
                    val = val.strip().strip('"\'')
                    config[key] = val

    content = f"""// Auto-generated from .env by scripts/load_env.py
// DO NOT EDIT MANUALLY - EDIT .env INSTEAD
#pragma once

#define WIFI_SSID "{config.get('WIFI_SSID', '')}"
#define WIFI_PASSWORD "{config.get('WIFI_PASSWORD', '')}"
#define TIMEZONE_STR "{config.get('TIMEZONE', 'PST8PDT,M3.2.0,M11.1.0')}"
#define NTP_SERVER_STR "{config.get('NTP_SERVER', 'pool.ntp.org')}"
"""
    if os.path.isfile(out_file):
        with open(out_file, 'r', encoding='utf-8') as f:
            old_content = f.read()
        if old_content == content:
            return
            
    with open(out_file, 'w', encoding='utf-8') as f:
        f.write(content)
    print(f"[load_env.py] Generated {out_file} from .env")

load_env_file()
