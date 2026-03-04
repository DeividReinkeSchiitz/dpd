## latexmk configuration: place aux and output files in build/
$aux_dir = 'build/';
$out_dir = 'build/';

# Ensure pdflatex is used
$pdf_mode = 1;

# extensions to clean
@clean_ext = ('aux','bbl','blg','brf','fdb_latexmk','fls','idx','ilg','ind','lof','lot','log','out','toc','synctex.gz');
