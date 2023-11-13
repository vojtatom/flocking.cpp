import React from "react";
import ReactDOM from "react-dom/client";
import { FlockingPostPage } from "./components/Flocking.tsx";

import "./index.css";

ReactDOM.createRoot(document.getElementById("root")!).render(
    <React.StrictMode>
        <FlockingPostPage />
    </React.StrictMode>
);
