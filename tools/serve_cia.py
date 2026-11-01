"""Serve exactly one CIA to FBI over a local interface; produce its QR code.
No GitHub credentials are read, and no directory browsing is provided.
"""
import argparse, hashlib, ipaddress, json, secrets, time
from http.server import ThreadingHTTPServer, BaseHTTPRequestHandler
from pathlib import Path
import qrcode

def main():
 p=argparse.ArgumentParser();p.add_argument('--file',type=Path,required=True);p.add_argument('--host',required=True);p.add_argument('--port',type=int,default=8765);p.add_argument('--output',type=Path,required=True);p.add_argument('--minutes',type=int,default=1440)
 args=p.parse_args();file=args.file.resolve();assert file.is_file() and file.suffix.lower()=='.cia'
 host=ipaddress.ip_address(args.host);assert host.is_private and not host.is_loopback and not host.is_unspecified,'Choose this PC\'s LAN address'
 route='/'+secrets.token_urlsafe(18)+'/Java3DS.cia';url=f'http://{host}:{args.port}{route}';size=file.stat().st_size
 class Handler(BaseHTTPRequestHandler):
  def do_HEAD(self):self.send_file(False)
  def do_GET(self):self.send_file(True)
  def log_message(self,fmt,*values):pass
  def send_file(self,body):
   if not ipaddress.ip_address(self.client_address[0]).is_private or self.path!=route:self.send_error(404);return
   start,end=0,size-1;partial=False
   range_=self.headers.get('Range')
   if range_:
    try:
     assert range_.startswith('bytes=') and ',' not in range_
     first,last=range_[6:].split('-',1)
     if first:start=int(first);end=int(last) if last else end
     else:start=max(0,size-int(last))
     assert 0<=start<=end<size;partial=True
    except (ValueError,AssertionError):
     self.send_response(416);self.send_header('Content-Range',f'bytes */{size}');self.send_header('Content-Length','0');self.end_headers();return
   self.send_response(206 if partial else 200);self.send_header('Content-Type','application/octet-stream');self.send_header('Content-Disposition','attachment; filename="Java3DS.cia"');self.send_header('Content-Length',str(end-start+1));self.send_header('Accept-Ranges','bytes');self.send_header('Cache-Control','no-store')
   if partial:self.send_header('Content-Range',f'bytes {start}-{end}/{size}')
   self.end_headers()
   if body:
    try:
     with file.open('rb') as src:
      src.seek(start);remaining=end-start+1
      while remaining:
       block=src.read(min(65536,remaining))
       if not block:break
       self.wfile.write(block);remaining-=len(block)
    except (BrokenPipeError,ConnectionResetError):pass
 server=ThreadingHTTPServer((str(host),args.port),Handler);server.timeout=1;server.daemon_threads=True
 args.output.mkdir(parents=True,exist_ok=True)
 qr=qrcode.QRCode(error_correction=qrcode.constants.ERROR_CORRECT_M,box_size=8,border=4);qr.add_data(url);qr.make(fit=True);qr.make_image(fill_color='black',back_color='white').save(args.output/'Java3DS-install-QR.png')
 (args.output/'QR-address.txt').write_text(url+'\n')
 (args.output/'qr-server.json').write_text(json.dumps({'url':url,'file':str(file),'sha256':hashlib.sha256(file.read_bytes()).hexdigest(),'expires_at':time.time()+args.minutes*60},indent=2))
 print('QR installer ready on local network',flush=True)
 deadline=time.monotonic()+args.minutes*60
 try:
  while time.monotonic()<deadline:server.handle_request()
 finally:server.server_close()
if __name__=='__main__':main()
