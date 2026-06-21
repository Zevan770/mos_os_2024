-- if true then return {} end
return {
	"folke/todo-comments.nvim",
	---@module "todo-comments"
	---@type TodoOptions
	opts = {
		keywords = {
			exercise = {
				icon = " ",
				color = "default",
				alt = { [[Exercise]] },
			},
		},
	},
}
