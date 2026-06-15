{ stdenv, clang }:
stdenv.mkDerivation {
  pname = "hyperbox";
  version = "1.0";

  src = ./.;

  hardeningDisable = [ "stackprotector" ];

  buildPhase = ''
    ${clang}/bin/clang -O3 -s -Wall -static -nostdlib -ffreestanding -fno-math-errno -fno-trapping-math -freciprocal-math -march=native -fassociative-math -fomit-frame-pointer main.c -o hyperbox
  '';

  installPhase = ''
    mkdir -p $out/bin
    cp hyperbox $out/bin/hyperbox
    
    ln -s hyperbox $out/bin/cat
    ln -s hyperbox $out/bin/echo
    ln -s hyperbox $out/bin/mkdir
    ln -s hyperbox $out/bin/wc
    ln -s hyperbox $out/bin/cp
  '';
}
