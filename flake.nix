{
	description = "Dwall nix flake.";

	inputs = {
		nixpkgs.url = github:NixOS/nixpkgs/nixpkgs-unstable;
		systems.url = github:nix-systems/default;
	};

	outputs = { self, nixpkgs, systems }: let
		eachSystem = nixpkgs.lib.genAttrs (import systems);
	in {
		packages = eachSystem (system: let
			pkgs = import nixpkgs { inherit system; };
			dwall = debug: let
				builder = rec {
					customCflags = pkgs.lib.strings.trim (builtins.readFile (if debug then ./compile_flags_debug.txt else ./compile_flags.txt));
					cc = "${pkgs.clang}/bin/clang -c ${customCflags}";
					ld = "${pkgs.clang}/bin/clang";
				};
			in pkgs.stdenv.mkDerivation {
				name = "dwall";
				version = "0.1.0";

				nativeBuildInputs = [ pkgs.pkgsStatic.curl.dev ];

				dontPatch = true;
				dontConfigure = true;
				dontUnpack = true;
				dontStrip = debug;
				hardeningDisable = if debug then [ "all" ] else [];
				buildPhase = ''
					export CFLAGS=$(curl-config --cflags)
					export LDFLAGS=$(curl-config --libs)
					mkdir -p build

					find ${ ./src } -type f -name "*.c" | while read f; do
						${builder.cc} $CFLAGS $f -o "build/$(basename $f .c).o"
					done

					${builder.ld} build/*.o -o build/dwall $LDFLAGS

					mkdir -p $out/bin
					cp build/dwall $out/bin/dwall
				'';
			};
		in {
			dwall = dwall false;
			dwall-debug = dwall true;
			default = dwall false;
		} );
		devShells = eachSystem (system: let
			pkgs = import nixpkgs { inherit system; };
		in rec {
			default = dwall;
			dwall = pkgs.mkShell {
				buildInputs = [ pkgs.pkgsStatic.curl.dev ];
			};
		} );
	};
}
