import React from 'react';
import { Demo } from './demo';
import { Link } from 'react-router-dom';

export const Home = () => {
  return (
    <div
      style={{
        width: '100%',
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        overflow: 'hidden',
      }}
    >
      <Demo />
      <div
        style={{
          width: 'min(800px, 100%)',
          boxSizing: 'border-box',
          padding: '0 20px',
          margin: 'auto',
        }}
      >
        <h1>Otto Simulator</h1>
        <p>
          This project brings <a href="https://www.ottodiy.com/">Otto</a> to your browser.
          Even better, it allows you to simulate Otto within Python.
          The simulation implements the Gym Interface.
        </p>
        <h2>Run the Simulation</h2>
        <p>
          The animation above is a direct result of the simulation.<br />
          To run a specific agent live in the browser open the <Link to="/otto-simulator/editor/">Editor</Link>.<br />
          To control Otto with a keyboard or gamepad visit the <Link to="/otto-simulator/play/">Play</Link> section.
        </p>
        <h2>Open Source</h2>
        <p>
          This project is Open-Source. The source is available on <a href="https://github.com/szabolcsdombi/otto-simulator">GitHub</a>.
        </p>
        <h2>Technical Details</h2>
        <p>
          This website is built using React and deployed directly to GitHub Pages.<br />
          The landing page animation is a direct port of the renderer to WebGL2, the animation frames are exported from the simulation. This enables realistic visualization with an almost zero loading time.<br />
          The simulation uses the MultiBody system with a FeatherStone solver of the <a href="https://github.com/bulletphysics/bullet3">bulletphysics</a> library.<br />
          An extension enables the Python langunage to interact with the simulation implemented in C++.<br />
          A local dev environment can be used for Training and Evaluation just like any other Gym Envs. This requires the entire toolset to be installed locally.<br />
          For those who wish to try it with no installation an online version was built using <a href="https://github.com/pyodide/pyodide">Pyodide</a>.<br />
          The visualization is implemented with <a href="https://github.com/szabolcsdombi/zengl">ZenGL</a> directly in Python. The browser version uses a WebGL2 backend.
        </p>
        <h2>Gym Interface</h2>
        <p>
          Implementing the Gym interface for the simulation provides several benefits:
        </p>
        <ul>
          <li><span className="title">Standardized interface:</span> The Gym interface is widely used in the field of reinforcement learning and robotics. By implementing the Gym interface, the simulation can be easily integrated with existing RL algorithms and frameworks, making it more accessible to researchers and developers.</li>
          <li><span className="title">Reinforcement learning compatibility:</span> The Gym interface allows reinforcement learning agents to interact with the simulation environment using a standardized set of actions and observations. This compatibility enables researchers to apply various RL algorithms to train the robot in the simulation environment.</li>
          <li><span className="title">Benchmarking and comparison:</span> The Gym interface provides a standardized way to define reward functions, episode termination conditions, and other evaluation metrics. This allows researchers to benchmark different algorithms and compare their performance on the same simulation environment. It promotes fair comparisons and facilitates the exchange of results and techniques within the community.</li>
          <li><span className="title">Reproducibility:</span> By adhering to the Gym interface, the simulation environment becomes reproducible. This means that other researchers and developers can easily reproduce and validate the results obtained using the simulation, as they can use the same Gym-compatible codebase and configurations.</li>
          <li><span className="title">Community support:</span> The Gym interface has a large community of developers and researchers actively working on reinforcement learning and robotics. By implementing the Gym interface, the simulation gains access to this community and can benefit from the shared knowledge, code libraries, and collaborations.</li>
        </ul>
        <p>
          Overall, implementing the Gym interface for the simulation enhances its compatibility, accessibility, and reproducibility while fostering collaboration and advancements in the field of reinforcement learning and robotics.
        </p>
      </div>
    </div>
  );
};
