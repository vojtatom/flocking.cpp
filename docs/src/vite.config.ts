import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";
import { ViteImageOptimizer } from "vite-plugin-image-optimizer";

// https://vitejs.dev/config/
export default defineConfig({
    base: "/flocking.cpp",
    plugins: [
        react(),
        ViteImageOptimizer({
            png: {
                quality: 50,
            },
            webp: {
                quality: 50,
            },
            jpeg: {
                quality: 50,
            },
        }),
    ],
});
