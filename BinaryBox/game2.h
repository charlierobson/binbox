extern int gt;
extern void outputToModule(byte, byte);
extern void outputHex(byte);
extern byte collectBinary(void);
extern void outputBinary(byte);
extern bool actFlipped(void);
extern byte randy;

void* game2_loop(void);
void* game2_no(void);
void* game2_yes(void);

void* game2_begin(void)
{
  int binary = collectBinary();
  outputBinary(binary);
  
  randy = random(256);

  outputHex(randy);

  outputToModule(0x80, 0x20);
  outputToModule(0x40, 0x20);
  outputToModule(0x20, 0x20);

  return (void*)game2_loop;
}

void* game2_loop()
{
  int binary = collectBinary();
  outputBinary(binary);

  if (actFlipped())
  {
    gt = 0;
    if (binary == randy)
    {
      return (void*)game2_yes;
    }
    return (void*)game2_no;
  }
  return (void*)game2_loop;
}

void* game2_no()
{
  outputToModule(0x80, 0);
  outputToModule(0x40, 0);
  outputToModule(0x20, 0);

  if ((gt & 128) == 0)
  {
    outputToModule(0x10, 0x70);
    outputToModule(0x08, 0x71);
  }
  else
  {
    outputToModule(0x10, 0);
    outputToModule(0x08, 0);
  }

  if (gt < 3000) return (void*)game2_no;

  outputHex(randy);

  outputToModule(0x10, 0x20);
  outputToModule(0x08, 0x20);

  return (void*)game2_loop;
}

void* game2_yes()
{
  outputToModule(0x10, 0);
  outputToModule(0x08, 0);

  if ((gt & 256) == 0)
  {
    outputToModule(0x80, 0b01101010);
    outputToModule(0x40, 0b10110011);
    outputToModule(0x20, 0b11100011);
  }
  else
  {
    outputToModule(0x80, 0);
    outputToModule(0x40, 0);
    outputToModule(0x20, 0);
  }

  if (gt < 4000) return (void*)game2_yes;

  return (void*)game2_begin;
}

