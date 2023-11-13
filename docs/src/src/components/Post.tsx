import React from "react";
import { Helmet } from "react-helmet";

import "./Post.css";

interface ProjectPageProps {
    children: React.ReactNode | React.ReactNode[];
    clickableImages?: boolean;
    title: string;
}

export function Post(props: ProjectPageProps) {
    React.useEffect(() => {
        if (props.clickableImages) {
            let imgs = document.getElementsByTagName("img");
            for (let i of imgs) {
                i.onclick = () => {
                    let s = document.getElementsByTagName("body")[0];

                    let frame = document.createElement("div");
                    frame.classList.add("img-frame");

                    frame.onclick = () => {
                        frame.parentElement!.removeChild(frame);
                    };

                    let image = document.createElement("img");
                    image.src = i.src;

                    frame.appendChild(image);
                    s.appendChild(frame);
                };
            }
        }
    }, [props.clickableImages]);

    React.useLayoutEffect(() => {
        window.scrollTo(0, 0);
    });

    return (
        <>
            <Helmet>
                <meta charSet="utf-8" />
                <title>{props.title}</title>
            </Helmet>
            <div
                id="projectPost"
                className={props.clickableImages ? "clickableImages" : ""}
            >
                {props.children}
            </div>
            <div id="footer">&copy; {new Date().getFullYear()} vojtatom.cz</div>
        </>
    );
}
