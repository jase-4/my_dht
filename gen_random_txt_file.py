import random
import string

def generate_text_bytes(size_in_bytes):
    chars = string.ascii_letters + string.digits
    text = ''.join(random.choice(chars) for _ in range(size_in_bytes))
    return text

def save_text_to_file(filename, text_bytes):
    with open(filename, 'wb') as f:
        f.write(text_bytes)


length = 700
text = generate_text_bytes(length)
text_bytes = text.encode('utf-8') 



filename = f'random{length}.txt'
save_text_to_file(filename, text_bytes)

print(f"Generated text saved to {filename}")