-- HACK: always trust after editing this file **inside neovim**
local cur_file = debug.getinfo(1, "S").source:sub(2)
vim.api.nvim_create_autocmd("BufWrite", {
	pattern = cur_file,
	callback = function(args)
		vim.secure.trust({ action = "allow", bufnr = args.buf })
	end,
})

vim.lsp.config("ctags_lsp", {
	cmd = { "ctags-lsp" },
	filetypes = { "ruby", "c" },
})

vim.lsp.enable("ctags_lsp")
vim.lsp.enable("clangd", false)
