#!/usr/bin/env python3
"""Pure offline media tests; no USB, printer, live CUPS or proprietary GPD required."""
import json,os,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
CAT=json.loads((ROOT/'driver/media/media.json').read_text())
assert len(CAT)==86 and len({m['name'] for m in CAT})==86
assert len(CAT[:43])==len(CAT[43:])==43
assert all(not m['name'].startswith('Leitz') for m in CAT[:43])
assert [m['name'] for m in CAT[43:]]==['Leitz'+m['name'] for m in CAT[:43]]
assert [m['name'].lower() for m in CAT[:43]]==sorted(m['name'].lower() for m in CAT[:43])
assert next(m for m in CAT if m['name']=='LeverArchFile1005')['page']==next(m for m in CAT if m['name']=='LeitzLeverArchFile1005')['page']
assert [m['name'] for m in CAT if m['name'].startswith('Leitz') and m['name'][5:] not in {x['name'] for x in CAT[:43]}]==[]
assert {a['name'] for a in CAT[:43] if any(a.get(field)!=next(x for x in CAT if x['name']=='Leitz'+a['name']).get(field) for field in ('page','area','origin','command_length','command_width','continuous'))}=={'LargePendaflexTabRotated','Multipurpose70030001Rotated','NameBadgeCard70190001Rotated','ShippingLabel70180001Rotated'}
print('PASS 43 A + 43 B media profiles; four vendor geometry differences preserved')
ppd=(ROOT/'driver/cups/LeitzIcon.ppd').read_text()
for m in CAT:
  name=m['name'];assert f'*PageSize {name}/' in ppd and f'*PageRegion {name}:' in ppd
  assert f'*ImageableArea {name}:' in ppd and f'*PaperDimension {name}:' in ppd
assert '*PageSize Continuous70030001/Continuous - 3 1/2"' in ppd
assert '*PageSize StandardAddressLabel/Standard Address Label:' in ppd
assert '*PageSize ShippingLabel70180001/Shipping Label 2" x 3 1/2"' in ppd
assert '*PageSize Continuous70070001/Continuous - 1 1/2"' in ppd
assert '*Resolution 300x300dpi/300 x 300 dpi:' in ppd
assert '*OpenUI *LeitzProtocol/Print Protocol (development): PickOne' in ppd
assert '*DefaultLeitzProtocol: Auto' in ppd
assert '*LeitzProtocol Auto/Automatic (Current, then Native): ""' in ppd
assert '*DefaultLabelCutter: ON' in ppd
assert '*ColorModel Gray8/8-bit Grayscale (driver dither):' in ppd
assert '*LeitzDither Bayer4/Ordered 4x4 Bayer:' in ppd
assert '*LeitzDither Threshold/Black/White Threshold:' in ppd
assert '*LabelCutter OFF/Cut At Job End (experimental):' in ppd
assert '*LeitzProtocol Native/Native: ""' in ppd
print('PASS vendor-readable CUPS media labels and confirmed 300x300 dpi option')
assert len([line for line in ppd.splitlines() if line.startswith('*PageSize ')])==86
assert [line.split()[1].split('/')[0] for line in ppd.splitlines() if line.startswith('*PageSize ')]==[m['name'] for m in CAT]
assert '*PageSize LeitzWristband/*Wristband:' in ppd
assert '*PageSize LeitzLeverArchFile1005/*Lever Arch File 1005:' in ppd
print('PASS 86 A-then-B PPD sizes with source-matched original Windows B labels')

with tempfile.TemporaryDirectory() as t:
 tmp=Path(t)
 def run_case(mname, dpi=600, empty=False, native=False, pages=1, cut_each=True):
  m=next((x for x in CAT if x['name']==mname),None)
  w=m['area'][0] if m else 960
  h=(((m['area'][1]+1)//2 if dpi==300 else m['area'][1])) if m else 1120
  if h>65000:raise AssertionError('too long')
  pgm=tmp/'media.pgm'
  with pgm.open('wb') as f:
   for _page in range(pages):
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
      'LeitzCutMode=Automatic' + (' LeitzProtocol=Native' if native else '') + (' LabelCutter=OFF' if not cut_each else ''),str(pgm)],env=env,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
  if empty:
   assert result.returncode!=0 and not result.stdout
   print('PASS blank input blocked');return
  assert result.returncode==0, (mname,dpi,result.stderr.decode()[-700:])
  out=tmp/'out.bin';out.write_bytes(result.stdout)
  status=subprocess.run([str(ROOT/'leitz-cli'),'validate-native' if native else 'validate',str(out)],capture_output=True)
  assert status.returncode==0,(mname,status.stderr)
  assert (f'{pages} validated label(s)' in status.stderr.decode() if not native else f'pages={pages};' in status.stderr.decode()), status.stderr
  if native:
    assert result.stdout[:4] in (b'\x1b@\x1bi',b'\x1b@\x1bh')
    if m['continuous']:
      # Original Linux 1.0.3: trailing blank lines stay pending.
      # The common controller must not insert native-only feed commands.
      bottom=181 if dpi==600 else 91
      assert result.stdout.endswith(b'\x1bG\x1bE'),result.stdout[-20:]
      assert not result.stdout.endswith(
          b'\x1bf' + bottom.to_bytes(2,'big') + b'\x1bG\x1bE')
    width=(result.stdout[130]<<8)|result.stdout[131]
    other=subprocess.run([str(ROOT/'leitz-cli'),'validate',str(out)],capture_output=True)
    assert other.returncode!=0,other.stderr
  else:
    preamble=result.stdout[120:135]
    width=(preamble[-2]<<8)|preamble[-1]
  assert width==w,(mname,width,w)
  assert (b'protocol=Native' if native else b'protocol=Current') in result.stderr
  if m['continuous']:
   assert b'cut=after-content' in result.stderr
   assert len(result.stdout)<2000,len(result.stdout)
  else:assert b'cut=page' in result.stderr
  print(f'PASS {mname} @300x{dpi} width={w} payload={len(result.stdout)}')
  return result.stdout
 for name in ('Multipurpose70030001','WideLeverArchLabelShort','NameBadgeLabel70060001',
              'NarrowLeverArchLabelShort','ReturnAddressLabelSmall',
              'LeitzFileFolderLabel','Continuous70090001','Continuous70030001'):
  run_case(name)
 run_case('Multipurpose70030001',300)
 for name in ('LargePendaflexTabRotated','Multipurpose70030001Rotated','NameBadgeCard70190001Rotated','ShippingLabel70180001Rotated'):
  run_case(name)
 run_case('LargePendaflexTabRotated',300)
 run_case('Continuous112')
 # Every new alias must run through the same CUPS raster-to-USB-byte pipeline.
 for name in ('LeverArchFile1005','LeitzLeverArchFile1005',
              'DHLShippingLabel','LeitzDHLShippingLabel',
              'NameBadgeCard8214','LeitzNameBadgeCard8214',
              'Wristband','LeitzWristband',
              'LargePendaflexTabRotated','LeitzLargePendaflexTabRotated',
              'Multipurpose70030001Rotated','LeitzMultipurpose70030001Rotated',
              'NameBadgeCard70190001Rotated','LeitzNameBadgeCard70190001Rotated',
              'ShippingLabel70180001Rotated','LeitzShippingLabel70180001Rotated'):
  run_case(name)
 run_case('Multipurpose70030001',empty=True)
 # Exercise the actual CUPS filter selection: native 1.0.3 emits a distinct
 # vendor framing, but the tested current path remains the default.
 run_case('FileFolderLabel',native=True)
 run_case('FileFolderLabel',native=True,dpi=300)
 run_case('Continuous70030001',native=True)
 run_case('Continuous70030001',native=True,dpi=300)
 # Both codecs must honor the same job-level cutter and page-count choices.
 for use_native in (False,True):
  one_cut=run_case('FileFolderLabel',native=use_native)
  one_end=run_case('FileFolderLabel',native=use_native,cut_each=False)
  assert one_cut==one_end, 'Single-label bytes must be unchanged by cut policy'
  two_cut=run_case('FileFolderLabel',native=use_native,pages=2)
  two_end=run_case('FileFolderLabel',native=use_native,pages=2,cut_each=False)
  assert two_cut!=two_end, 'Inter-label cut must differ'
  assert two_end.count(b'\x1bE')==1, 'Only final cut expected'
 # Exercise the real filter's 8-bit grayscale path with continuous/custom
 # media. The binary 1-bit default above must remain byte-identical.
 gradient=tmp/'gradient.pgm'
 gradient.write_bytes(b'P5\n32 32\n255\n' + (b'\x00'*32) + (b'\x80'*32)*31)
 def gray_job(dither,space_w=False,native=False):
  env=os.environ.copy();env.update(MOCK_MEDIA_NAME='Custom',MOCK_GRAY8='1')
  if space_w:env['MOCK_COLORSPACE_W']='1'
  options='PageSize=Custom ColorModel=Gray8 LeitzDither='+dither
  if native:options+=' LeitzProtocol=Native'
  p=subprocess.run([str(ROOT/'leitz-rastertoicon-mock'),'1','root','Grayscale','1',options,str(gradient)],
      env=env,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
  assert p.returncode==0,p.stderr.decode()
  binary=tmp/'gray.bin';binary.write_bytes(p.stdout)
  check=subprocess.run([str(ROOT/'leitz-cli'),'validate-native' if native else 'validate',str(binary)],
      capture_output=True)
  assert check.returncode==0,check.stderr
  return p.stdout
 for native in (False,True):
  ordered=gray_job('Bayer4',native=native)
  threshold=gray_job('Threshold',native=native)
  assert ordered!=threshold,'Halftone setting must change the raster'
  assert ordered==gray_job('Bayer4',space_w=True,native=native),'K/W polarity mismatch'
  assert threshold==gray_job('Threshold',space_w=True,native=native),'K/W threshold mismatch'
 print('PASS grayscale 8-bit Bayer4/threshold, K/W polarity, both wire protocols')
 print('PASS offline media checks for both actual print protocols')
