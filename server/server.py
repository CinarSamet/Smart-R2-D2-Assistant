from flask import Flask, request, Response
import whisper
import edge_tts
import google.generativeai as genai
from flask import Flask, request, Response, send_file 
import os, subprocess, uuid, asyncio
import json

with open("config/settings.json") as f:
    config = json.load(f)

tts_config = config["tts"]
server_config = config["server"]
ai_config = config["ai"]

UPLOAD_DIR = server_config["records_dir"]
TTS_DIR = server_config["tts_dir"]
os.makedirs(UPLOAD_DIR, exist_ok=True)
os.makedirs(TTS_DIR, exist_ok=True)

app = Flask(__name__)

print("Whisper yükleniyor...")
model = whisper.load_model("turbo")

genai.configure(api_key=os.getenv("GEMINI_API_KEY"))
gemini_model = genai.GenerativeModel(ai_config["model"])


#  AUDIO 
def transcribe_audio(raw_bytes):
    raw_path = f"{UPLOAD_DIR}/{uuid.uuid4().hex}.raw"
    wav_path = raw_path.replace(".raw", ".wav")

    with open(raw_path, "wb") as f:
        f.write(raw_bytes)

    subprocess.run([
        "ffmpeg", "-y",
        "-f", "s16le", "-ar", "16000", "-ac", "1",
        "-i", raw_path, wav_path
    ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    try:
        result = model.transcribe(wav_path, language="tr", fp16=False)
        return result.get("text", "").strip()
    except:
        return ""


def ask_ai(text):
    if not text:
        return "Orada mısın"

    prompt = f"""
{ai_config["prompt"]}
{text}
"""
    try:
        r = gemini_model.generate_content(prompt)
        return r.text.strip()
    except Exception as e:
        print("Gemini hatası:", e)
        return "Token bitti"


async def tts_async(text, out_wav):
    tts = edge_tts.Communicate(
        text=text,
        voice=tts_config["voice"],
        rate=tts_config["rate"],
        pitch=tts_config["pitch"]
    )
    await tts.save(out_wav)


def speak_to_raw(text):
    wav = f"{TTS_DIR}/{uuid.uuid4().hex}.wav"
    raw = wav.replace(".wav", ".raw")

    asyncio.run(tts_async(text, wav))

    if not os.path.exists(wav):
        return None

    subprocess.run([
        "ffmpeg", "-y",
        "-i", wav,
        "-f", "s16le", "-ar", "16000", "-ac", "1",
        raw
    ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    return raw if os.path.exists(raw) else None


def cleanup_old_files(folder, max_files=50):
    files = sorted(
        [os.path.join(folder, f) for f in os.listdir(folder)],
        key=os.path.getmtime
    )
    while len(files) > max_files:
        os.remove(files.pop(0))

@app.route("/upload", methods=["POST"])
def upload():
    raw_bytes = request.data
    if not raw_bytes:
        print("Uyari: Bos veri geldi.")
        return Response(b"", mimetype="application/octet-stream")

    print(f"ESP32'dan gelen ses boyutu: {len(raw_bytes)} bytes")

    text = transcribe_audio(raw_bytes)
    print(f"Algilanan metin: '{text}'")

    reply = ask_ai(text)
    print(f"AI cevabi: '{reply}'")

    raw_audio = speak_to_raw(reply)
    if raw_audio is None:
        print("TTS -> PCM dönüşümü başarısız")
        return Response(b"", mimetype="application/octet-stream")

    print("PCM dosyası ESP32'ye gönderiliyor")
    return send_file(raw_audio, mimetype="application/octet-stream")

if __name__ == "__main__":
    app.run(host=server_config["server_host"], port=server_config["server_port"])