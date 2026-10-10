#!/usr/bin/env python3
"""Pure offline media tests; no USB, printer, live CUPS or proprietary GPD required."""
import json,os,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CAT=json.loads((ROOT/'media.json').read_text())
assert len(CAT)==81 and len({m['name'] for m in CAT})==81
ppd=(ROOT/'cups/LeitzIcon.ppd').read_text()
for m in CAT:
  name=m['name'];assert f'*PageSize {name}/' in ppd and f'*PageRegion {name}:' in ppd
  assert f'*ImageableArea {name}:' in ppd and f'*PaperDimension {name}:' in ppd
assert '*PageSize Continuous70030001/Continuous - 3 1/2"' in ppd
assert '*PageSize StandardAddressLabel/Standard Address Label:' in ppd
assert '*PageSize ShippingLabel70180001/Shipping Label 2" x 3 1/2"' in ppd
assert '*PageSize Continuous70070001/Continuous - 1 1/2"' in ppd
assert '*Resolution 300x300dpi/300 x 300 dpi:' in ppd
print('PASS vendor-readable CUPS media labels and confirmed 300x300 dpi option')
print('PASS 76 Windows GPD and 5 original Linux/macOS media entries exposed')

with tempfile.TemporaryDirectory() as t:
 tmp=Path(t)
 def run_case(mname, dpi=600, empty=False):
  m=next((x for x in CAT if x['name']==mname),None)
  w=m['area'][0] if m else 960
  h=(((m['area'][1]+1)//2 if dpi==300 else m['area'][1])) if m else 1120
  if h>65000:raise AssertionError('too long')
  pgm=tmp/'media.pgm'
  with pgm.open('wb') as f:
   f.write(f'P5\n{w} {h}\n255\n'.encode())
   row=bytearray(b'\xff'*w)
   for y in range(h):
    row[:]=b'\xff'*w
    if not empty and y in (15,120):
     row[min(15 if y==15 else 50,w-1)]=0
    f.write(row)
  env=os.environ.copy();env['MOCK_MEDIA_NAME']=mname
  if dpi==300:env['MOCK_DPI300']='1'
  result=subprocess.run([str(ROOT/'leitz-rastertoicon-mock'),'1','root','Test','1',
      'LeitzCutMode=Automatic',str(pgm)],env=env,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
  if empty:
   assert result.returncode!=0 and not result.stdout
   print('PASS blank input blocked');return
  assert result.returncode==0, (mname,dpi,result.stderr.decode()[-700:])
  out=tmp/'out.bin';out.write_bytes(result.stdout)
  status=subprocess.run([str(ROOT/'leitz-cli'),'validate',str(out)],capture_output=True)
  assert status.returncode==0,(mname,status.stderr)
  preamble=result.stdout[120:135]
  width=(preamble[-2]<<8)|preamble[-1]
  assert width==w,(mname,width,w)
  if m['continuous']:
   assert b'cut=after-content' in result.stderr
   assert len(result.stdout)<2000,len(result.stdout)
  else:assert b'cut=page' in result.stderr
  print(f'PASS {mname} @300x{dpi} width={w} payload={len(result.stdout)}')
 for name in ('Multipurpose70030001','WideLeverArchLabelShort','NameBadgeLabel70060001',
              'NarrowLeverArchLabelShort','ReturnAddressLabelSmall',
              'LeitzFileFolderLabel','Continuous70090001','Continuous70030001'):
  run_case(name)
 run_case('Multipurpose70030001',300)
 for name in ('LargePendaflexTabRotated','Multipurpose70030001Rotated','NameBadgeCard70190001Rotated','ShippingLabel70180001Rotated'):
  run_case(name)
 run_case('LargePendaflexTabRotated',300)
 run_case('Multipurpose70030001',empty=True)
 print('PASS offline media checks')
