"""Build the update CA bundle from roots of locally VERIFIED TLS chains (Python 3.13+).
Never disables verification or stores leaf certificates as trust anchors.
Review root names/fingerprints before including the bundle in a release.
"""
import ssl,socket,hashlib
from pathlib import Path
r=Path(__file__).resolve().parents[1]
ctx=ssl.create_default_context();roots={}
for host in ('api.github.com','github.com','release-assets.githubusercontent.com','objects.githubusercontent.com'):
 with ctx.wrap_socket(socket.create_connection((host,443),timeout=15),server_hostname=host) as sock:
  cert=sock.get_verified_chain()[-1];digest=hashlib.sha256(cert).hexdigest();roots[digest]=ssl.DER_cert_to_PEM_cert(cert)
  print(host,digest)
p=r/'romfs/certs';p.mkdir(exist_ok=True)
(p/'github-roots.pem').write_text(''.join(roots.values()),encoding='ascii')
