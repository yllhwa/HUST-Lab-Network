import { defineConfig } from "vite";
import vue from "@vitejs/plugin-vue";
import Components from "unplugin-vue-components/vite";
import { NaiveUiResolver } from "unplugin-vue-components/resolvers";

export default defineConfig({
  plugins: [
    vue({
      reactivityTransform: true,
    }),
    Components({
      resolvers: [NaiveUiResolver()],
    }),
  ],
  define: {
    __VUE_OPTIONS_API__: false,
  },
  esbuild: {
    // 去除console
    minify: true,
    pure: ["console.log"],
  },
});
