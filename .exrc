if &cp | set nocp | endif
let s:cpo_save=&cpo
set cpo&vim
inoremap <silent> <Plug>(neocomplete_auto_refresh) =neocomplete#mappings#refresh()
inoremap <silent> <Plug>(neocomplete_start_manual_complete) =neocomplete#mappings#manual_complete()
inoremap <silent> <Plug>(neocomplete_start_auto_complete) =neocomplete#mappings#auto_complete()
inoremap <silent> <Plug>(neocomplete_start_omni_complete) 
inoremap <silent> <expr> <Plug>(neocomplete_start_unite_quick_match) unite#sources#neocomplete#start_quick_match()
inoremap <silent> <expr> <Plug>(neocomplete_start_unite_complete) unite#sources#neocomplete#start_complete()
inoremap <silent> <expr> <Plug>(neosnippet_start_unite_snippet) unite#sources#neosnippet#start_complete()
inoremap <silent> <expr> <Plug>(neosnippet_jump) neosnippet#mappings#jump_impl()
inoremap <silent> <expr> <Plug>(neosnippet_expand) neosnippet#mappings#expand_impl()
inoremap <silent> <expr> <Plug>(neosnippet_jump_or_expand) neosnippet#mappings#jump_or_expand_impl()
inoremap <silent> <expr> <Plug>(neosnippet_expand_or_jump) neosnippet#mappings#expand_or_jump_impl()
inoremap <expr> <BS> neocomplete#smart_close_popup()."\"
map! <D-v> *
snoremap  a<BS>
smap <expr> 	 neosnippet#expandable_or_jumpable() ? "\<Plug>(neosnippet_expand_or_jump)" : "\	"
xmap  <Plug>(neosnippet_expand_target)
smap  <Plug>(neosnippet_expand_or_jump)
snoremap  a<BS>
nnoremap  o
xnoremap  o
onoremap  o
xmap gx <Plug>NetrwBrowseXVis
nmap gx <Plug>NetrwBrowseX
snoremap <Del> a<BS>
snoremap <BS> a<BS>
xnoremap <silent> <Plug>(neosnippet_register_oneshot_snippet) :call neosnippet#mappings#_register_oneshot_snippet()
xnoremap <silent> <Plug>(neosnippet_expand_target) :call neosnippet#mappings#_expand_target()
xnoremap <silent> <Plug>(neosnippet_get_selected_text) :call neosnippet#helpers#get_selected_text(visualmode(), 1)
snoremap <silent> <expr> <Plug>(neosnippet_jump) neosnippet#mappings#jump_impl()
snoremap <silent> <expr> <Plug>(neosnippet_expand) neosnippet#mappings#expand_impl()
snoremap <silent> <expr> <Plug>(neosnippet_jump_or_expand) neosnippet#mappings#jump_or_expand_impl()
snoremap <silent> <expr> <Plug>(neosnippet_expand_or_jump) neosnippet#mappings#expand_or_jump_impl()
vnoremap <silent> <Plug>NetrwBrowseXVis :call netrw#BrowseXVis()
nnoremap <silent> <Plug>NetrwBrowseX :call netrw#BrowseX(expand((exists("g:netrw_gx")? g:netrw_gx : '<cfile>')),netrw#CheckIfRemote())
xmap <BS> "-d
vmap <D-x> "*d
vmap <D-c> "*y
vmap <D-v> "-d"*P
nmap <D-v> "*P
inoremap <expr>  neocomplete#undo_completion()
inoremap <expr>  neocomplete#smart_close_popup()."\"
inoremap <expr> 	 pumvisible() ? "\" : "\	"
imap  <Plug>(neosnippet_expand_or_jump)
inoremap <expr>  neocomplete#complete_common_string()
let &cpo=s:cpo_save
unlet s:cpo_save
set autoread
set backspace=indent,eol,start
set clipboard=unnamed,autoselect
set cmdheight=2
set completeopt=preview,menuone
set fileencodings=ucs-bom,utf-8,default,latin1
set helplang=ja
set hidden
set hlsearch
set ignorecase
set incsearch
set laststatus=2
set listchars=tab:^-,eol:↵
set matchpairs=(:),{:},[:],<:>
set matchtime=1
set ruler
set runtimepath=~/.cache/dein/repos/github.com/Shougo/dein.vim/,~/.vim,~/.cache/dein/repos/github.com/Shougo/vimproc.vim,~/.cache/dein/repos/github.com/Shougo/neosnippet-snippets,~/.cache/dein/repos/github.com/Shougo/neosnippet,~/.cache/dein/repos/github.com/Shougo/neocomplete.vim,~/.cache/dein/.cache/.vimrc/.dein,/usr/local/share/vim/vimfiles,/usr/local/share/vim/vim80,~/.cache/dein/.cache/.vimrc/.dein/after,/usr/local/share/vim/vimfiles/after,~/.vim/after
set shiftwidth=4
set showcmd
set showmatch
set showtabline=2
set smartcase
set smartindent
set noswapfile
set tabline=%!airline#extensions#tabline#get()
set tabstop=4
set visualbell
set wildmenu
set nowrapscan
" vim: set ft=vim :
