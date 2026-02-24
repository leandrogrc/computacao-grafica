from PIL import Image
import os

def process_weapon_image(input_path, output_path, should_crop=False):
    img = Image.open(input_path).convert("RGBA")
    
    # 1. Corrigir proporção (cortar para quadrado centrado)
    if should_crop:
        w, h = img.size
        new_size = min(w, h)
        left = (w - new_size) / 2
        top = (h - new_size) / 2
        right = (w + new_size) / 2
        bottom = (h + new_size) / 2
        img = img.crop((left, top, right, bottom))
        # Redimensionar para 640x640
        img = img.resize((1080, 1080), Image.Resampling.LANCZOS)

    datas = img.getdata()
    new_data = []

    for item in datas:
        r, g, b, a = item
        # Lógica de Chroma Key (Verde)
        # Se o verde for dominante e maior que um limiar
        if g > r * 1.1 and g > b * 1.1 and g > 60:
            new_data.append((0, 0, 0, 0)) # Transparente
        else:
            new_data.append(item)

    img.putdata(new_data)
    img.save(output_path, "PNG", optimize=True)

# Lista de arquivos para processar
# Apenas a arma padrão parece precisar de crop agora
assets = [
    ("assets/gun_default.png", True),
    ("assets/gun_fire1.png", False),
    ("assets/gun_fire2.png", False),
    ("assets/gun_reload1.png", False),
    ("assets/gun_reload2.png", False),
    ("assets/gun_sprint.png", False)
]

for asset_path, should_crop in assets:
    if os.path.exists(asset_path):
        print(f"Processando {asset_path} (crop={should_crop})...")
        process_weapon_image(asset_path, asset_path, should_crop)
    else:
        print(f"Arquivo {asset_path} não encontrado.")
