{
  stdenv,
  clang,
  hwdata,
}:
stdenv.mkDerivation {
  pname = "hyperbox";
  version = "1.0";

  src = ./.;

  hardeningDisable = ["stackprotector"];

  nativeBuildInputs = [clang];
  buildInputs = [hwdata];

  buildPhase = let
    cpu = stdenv.hostPlatform.gcc.arch or "x86-64";
  in ''
    ${clang}/bin/clang -march=${cpu} -DOUI_PATH="\"${hwdata}/share/hwdata/oui.txt\"" -DOUI_SIZE="$(stat -c%s "${hwdata}/share/hwdata/oui.txt")" -s -Wall -static -nostdlib -ffreestanding -fno-math-errno -fno-trapping-math -freciprocal-math -fassociative-math -fomit-frame-pointer main.c -o hyperbox
  '';

  installPhase = ''
    mkdir -p $out/bin
    cp hyperbox $out/bin/hyperbox

    ln -s hyperbox $out/bin/discord_snowflake_parse
    ln -s hyperbox $out/bin/terminal_gameoflife
    ln -s hyperbox $out/bin/password_gen
    ln -s hyperbox $out/bin/urlencode
    ln -s hyperbox $out/bin/urldecode
    ln -s hyperbox $out/bin/maccheck
    ln -s hyperbox $out/bin/mkdir
    ln -s hyperbox $out/bin/echo
    ln -s hyperbox $out/bin/tee
    ln -s hyperbox $out/bin/cat
    ln -s hyperbox $out/bin/wc
    ln -s hyperbox $out/bin/cp
  '';
}
